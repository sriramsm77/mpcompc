%{
/* cfg_parser.y -- general-purpose configuration file parser
   Copyright (C) 2007-2022 Free Software Foundation, Inc.

   GNU Mailutils is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3, or (at
   your option) any later version.

   GNU Mailutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <mailutils/argcv.h>
#include <mailutils/wordsplit.h>
#include <mailutils/nls.h>
#include <mailutils/cfg.h>
#include <mailutils/alloc.h>
#include <mailutils/errno.h>
#include <mailutils/error.h>
#include <mailutils/list.h>
#include <mailutils/iterator.h>
#include <mailutils/debug.h>
#include <mailutils/util.h>
#include <mailutils/cctype.h>
#include <mailutils/stream.h>
#include <mailutils/stdstream.h>
#include <mailutils/cidr.h>
#include <mailutils/yyloc.h>

int mu_cfg_parser_verbose;
static mu_list_t /* of mu_cfg_node_t */ parse_node_list; 
size_t mu_cfg_error_count;

static int _mu_cfg_errcnt;

int yylex ();

void _mu_line_begin (void);
void _mu_line_add (char *text, size_t len);
char *_mu_line_finish (void);

static int
yyerror (char *s)
{
  mu_error ("%s", s);
  mu_cfg_error_count++;
  return 0;
}

static mu_config_value_t *
config_value_dup (mu_config_value_t *src)
{
  if (!src)
    return NULL;
  else
    {
      /* FIXME: Use mu_opool_alloc */
      mu_config_value_t *val = mu_alloc (sizeof (*val));
      *val = *src;
      return val;
    }
}

static int
_node_set_parent (void *item, void *data)
{
  struct mu_cfg_node *node = item;
  node->parent = data;
  return 0;
}

static mu_cfg_node_t *
mu_cfg_alloc_node (enum mu_cfg_node_type type, struct mu_locus_range *loc,
		   const char *tag, mu_config_value_t *label,
		   mu_list_t nodelist)
{
  char *p;
  mu_cfg_node_t *np;
  size_t size = sizeof *np + strlen (tag) + 1;
  np = mu_alloc (size);
  np->type = type;
  mu_locus_range_init (&np->locus);
  mu_locus_range_copy (&np->locus, loc);
  p = (char*) (np + 1);
  np->tag = p;
  strcpy (p, tag);
  np->label = label;
  np->nodes = nodelist;
  np->parent = NULL;
  return np;
}

void
mu_cfg_free_node (mu_cfg_node_t *node)
{
  free (node->label);
  free (node);
}

#define node_type_str(t) (((t) == mu_cfg_node_statement) ? "stmt" : "param")

static void
debug_print_node (mu_cfg_node_t *node)
{
  if (mu_debug_level_p (MU_DEBCAT_CONFIG, MU_DEBUG_TRACE0))
    {
      if (node->type == mu_cfg_node_undefined)
	{
	  /* Stay on the safe side */
	  mu_error (_("unknown statement type!"));
	  mu_cfg_error_count++;
	}
      else
	{
	  /* FIXME: How to print label? */
	  mu_error ("statement: %s, id: %s",
		    node_type_str (node->type),
		    node->tag ? node->tag : "(null)");
	}
    }
}

static void
free_node_item (void *item)
{
  mu_cfg_node_t *node = item;

  switch (node->type)
    {
    case mu_cfg_node_statement:
      mu_list_destroy (&node->nodes);
      break;
      
    case mu_cfg_node_undefined: /* hmm... */
    case mu_cfg_node_param:
      break;
    }
  mu_cfg_free_node (node);
}

int
mu_cfg_create_node_list (mu_list_t *plist)
{
  int rc;
  mu_list_t list;

  rc = mu_list_create (&list);
  if (rc)
    return rc;
  mu_list_set_destroy_item (list, free_node_item);
  *plist = list;
  return 0;
}

%}

%define api.prefix {mu_cfg_yy}
%code requires {
#define MU_CFG_YYLTYPE struct mu_locus_range
#define yylloc mu_cfg_yylloc
#define yylval mu_cfg_yylval
}  
%locations
%expect 1

%union {
  mu_cfg_node_t node;
  mu_cfg_node_t *pnode;
  mu_list_t /* of mu_cfg_node_t */ nodelist;
  char *string;
  mu_config_value_t value, *pvalue;
  mu_list_t list;
}

%token <string> MU_TOK_IDENT MU_TOK_STRING MU_TOK_QSTRING MU_TOK_MSTRING
%type <string> string slist
%type <list> slist0
%type <value> value
%type <pvalue> tag vallist
%type <list> values list vlist
%type <string> ident
%type <nodelist> stmtlist
%type <pnode> stmt simple block

%%

input   : stmtlist
	  {
	    parse_node_list = $1;
	  }
	;

stmtlist: stmt
	  {
	    mu_cfg_create_node_list (&$$);
	    mu_list_append ($$, $1);
	  }
	| stmtlist stmt
	  {
	    mu_list_append ($1, $2);
	    $$ = $1;
	    debug_print_node ($2);
	  }
	;

stmt    : simple
	| block
	;

simple  : ident vallist ';'
	  {
	    struct mu_locus_range lr;
	    lr.beg = @1.beg;
	    lr.end = @3.end;
	    $$ = mu_cfg_alloc_node (mu_cfg_node_param, &lr, $1, $2, NULL);
	  }
	;

block   : ident tag '{' '}' opt_sc
	  {
	    struct mu_locus_range lr;
	    lr.beg = @1.beg;
	    lr.end = @5.end;
	    $$ = mu_cfg_alloc_node (mu_cfg_node_statement, &lr, $1, $2, NULL);
	  }
	| ident tag '{' stmtlist '}' opt_sc
	  {
	    struct mu_locus_range lr;
	    lr.beg = @1.beg;
	    lr.end = @6.end;
	    $$ = mu_cfg_alloc_node (mu_cfg_node_statement, &lr, $1, $2, $4);
	    mu_list_foreach ($4, _node_set_parent, $$);
	  }
	;

ident   : MU_TOK_IDENT
        ;

tag     : /* empty */
	  {
	    $$ = NULL;
	  }
	| vallist
	;

vallist : vlist
	  {
	    size_t n = 0;
	    mu_list_count($1, &n);
	    if (n == 1)
	      {
		mu_list_get ($1, 0, (void**) &$$);
	      }
	    else
	      {
		size_t i;
		mu_config_value_t val;

		val.type = MU_CFG_ARRAY;
		val.v.arg.c = n;
		/* FIXME: Use mu_opool_alloc */
		val.v.arg.v = mu_alloc (n * sizeof (val.v.arg.v[0]));
		if (!val.v.arg.v)
		  {
		    mu_error (_("not enough memory"));
		    abort();
		  }

		for (i = 0; i < n; i++)
		  {
		    mu_config_value_t *v;
		    mu_list_get ($1, i, (void **) &v);
		    val.v.arg.v[i] = *v;
		  }
		$$ = config_value_dup (&val);
	      }
	    mu_list_destroy (&$1);
	  }
	;

vlist   : value
          {
	    int rc = mu_list_create (&$$);
	    if (rc)
	      {
		mu_error (_("cannot create list: %s"), mu_strerror (rc));
		abort ();
	      }
	    mu_list_append ($$, config_value_dup (&$1)); /* FIXME */
	  }
	| vlist value
	  {
	    mu_list_append ($1, config_value_dup (&$2));
	  }
	;

value   : string
	  {
	      $$.type = MU_CFG_STRING;
	      $$.v.string = $1;
	  }
	| list
	  {
	      $$.type = MU_CFG_LIST;
	      $$.v.list = $1;
	  }
	| MU_TOK_MSTRING
	  {
	      $$.type = MU_CFG_STRING;
	      $$.v.string = $1;
	  }
	;

string  : MU_TOK_STRING
	| MU_TOK_IDENT
	| slist
	;

slist   : slist0
	  {
	    mu_iterator_t itr;
	    mu_list_get_iterator ($1, &itr);

	    _mu_line_begin ();
	    for (mu_iterator_first (itr);
		 !mu_iterator_is_done (itr); mu_iterator_next (itr))
	      {
		char *p;
		mu_iterator_current (itr, (void**)&p);
		_mu_line_add (p, strlen (p));
	      }
	    $$ = _mu_line_finish ();
	    mu_iterator_destroy (&itr);
	    mu_list_destroy(&$1);
	  }
	;

slist0  : MU_TOK_QSTRING
	  {
	    mu_list_create (&$$);
	    mu_list_append ($$, $1);
	  }
	| slist0 MU_TOK_QSTRING
	  {
	    mu_list_append ($1, $2);
	    $$ = $1;
	  }
	;

list    : '(' values ')'
	  {
	      $$ = $2;
	  }
	| '(' values ',' ')'
	  {
	      $$ = $2;
	  }
	;

values  : value
	  {
	    mu_list_create (&$$);
	    mu_list_append ($$, config_value_dup (&$1));
	  }
	| values ',' value
	  {
	    mu_list_append ($1, config_value_dup (&$3));
	    $$ = $1;
	  }
	;

opt_sc  : /* empty */
	| ';'
	;


%%

void
mu_cfg_set_debug ()
{
  if (mu_debug_level_p (MU_DEBCAT_CONFIG, MU_DEBUG_TRACE7))
    yydebug = 1;
}

int
mu_cfg_parse (mu_cfg_tree_t **ptree)
{
  int rc;
  mu_cfg_tree_t *tree;
  mu_opool_t pool;
  int save_mode = 0, mode;
  struct mu_locus_range save_locus = MU_LOCUS_RANGE_INITIALIZER;

  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, MU_IOCTL_LOGSTREAM_GET_MODE, 
                   &save_mode);
  mode = save_mode | MU_LOGMODE_LOCUS;
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, MU_IOCTL_LOGSTREAM_SET_MODE,
                   &mode);
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_GET_LOCUS_RANGE,
                   &save_locus);
  
  mu_cfg_set_debug ();
  _mu_cfg_errcnt = 0;

  rc = yyparse ();
  pool = mu_cfg_lexer_pool ();
  if (rc == 0 && _mu_cfg_errcnt)
    {
      mu_opool_destroy (&pool);
      rc = 1;
    }
  else
    {
      tree = mu_alloc (sizeof (*tree));
      tree->nodes = parse_node_list;
      tree->pool = pool;
      parse_node_list = NULL;
      *ptree = tree;
    }

  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, MU_IOCTL_LOGSTREAM_SET_MODE,
                   &save_mode);
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE,
                   &save_locus);
  mu_locus_range_deinit (&save_locus); /* FIXME: refcount? */

  return rc;
}

int
mu_cfg_tree_union (mu_cfg_tree_t **pa, mu_cfg_tree_t **pb)
{
  mu_cfg_tree_t *a, *b;
  int rc;
  
  if (!pb)
    return EINVAL;
  if (!*pb)
    return 0;
  b = *pb;
  if (!pa)
    return EINVAL;
  if (!*pa)
    {
      *pa = b;
      *pb = NULL;
      return 0;
    }
  else
    a = *pa;
  
  /* Merge opools */
  rc = mu_opool_union (&b->pool, &a->pool);
  if (rc)
    return rc;
    
  /* Link node lists */
  if (b->nodes)
    {
      mu_list_append_list (a->nodes, b->nodes);
      mu_list_destroy (&b->nodes);
    }
  
  free (b);
  *pb = NULL;
  return 0;
}

static mu_cfg_tree_t *
do_include (const char *name, struct mu_cfg_parse_hints *hints,
	    struct mu_locus_range const *loc)
{
  struct stat sb;
  char *tmpname = NULL;
  mu_cfg_tree_t *tree = NULL;
  
  if (name[0] != '/')
    {
      name = tmpname = mu_make_file_name (SYSCONFDIR, name);
      if (!name)
        {
          mu_error ("%s", mu_strerror (errno));
          return NULL;
        }
    }
  if (stat (name, &sb) == 0)
    {
      int rc = 0;
      
      if (S_ISDIR (sb.st_mode))
	{
	  if (hints->flags & MU_CFHINT_PROGRAM)
	    {
	      char *file = mu_make_file_name (name, hints->program);
	      rc = mu_cfg_parse_file (&tree, file, hints->flags);
	      free (file);
	    }
	  else
	    {
	      mu_diag_at_locus_range (MU_LOG_WARNING, loc,
				      _("ignoring `include': directory argument is allowed only from the top-level configuration file"));

	    }
	}
      else
	rc = mu_cfg_parse_file (&tree, name, hints->flags);
      
      if (rc == 0 && tree)
	{
	  struct mu_cfg_parse_hints xhints = *hints;
	  xhints.flags &= ~MU_CFHINT_PROGRAM;
	  mu_cfg_tree_postprocess (tree, &xhints);
	}
    }
  else if (errno == ENOENT)
    {
      mu_diag_at_locus_range (MU_LOG_ERROR, loc,
			      _("include file or directory does not exist"));
      mu_cfg_error_count++;
    }		   
  else
    {
      mu_diag_at_locus_range (MU_LOG_ERROR, loc,
			      _("cannot stat include file or directory: %s"),
			      mu_strerror (errno));
      mu_cfg_error_count++;
    }		   
  
  free (tmpname);
  return tree;
}

int
mu_cfg_tree_postprocess (mu_cfg_tree_t *tree, struct mu_cfg_parse_hints *hints)
{
  int rc;
  mu_iterator_t itr;
  
  if (!tree->nodes)
    return 0;
  rc = mu_list_get_iterator (tree->nodes, &itr);
  if (rc)
    return rc;
  for (mu_iterator_first (itr); !mu_iterator_is_done (itr);
       mu_iterator_next (itr))
    {
      mu_cfg_node_t *node;
      
      mu_iterator_current (itr, (void**) &node);
      
      if (node->type == mu_cfg_node_statement)
	{
	  if (strcmp (node->tag, "program") == 0)
	    {
	      if (hints->flags & MU_CFHINT_PROGRAM)
		{
		  if (node->label->type == MU_CFG_STRING)
		    {
		      if (strcmp (node->label->v.string, hints->program) == 0)
			{
			  /* Reset the parent node */
			  mu_list_foreach (node->nodes, _node_set_parent,
					   node->parent);
			  /* Move all nodes from this block to the topmost
			     level */
			  mu_iterator_ctl (itr, mu_itrctl_insert_list,
					   node->nodes);
			  mu_iterator_ctl (itr, mu_itrctl_delete, NULL);
			  /*FIXME:mu_cfg_free_node (node);*/
			}
		    }
		  else
		    {
		      mu_diag_at_locus_range (MU_LOG_ERROR, &node->locus,
					      _("argument to `program' is not a string"));
		      mu_cfg_error_count++;
		      mu_iterator_ctl (itr, mu_itrctl_delete, NULL);
		    }
		}
	      else
		{
		  mu_diag_at_locus_range (MU_LOG_WARNING, &node->locus,
					  _("ignoring `program' block: not located in top-level configuration file"));
		}
	    }
	}
      else if (node->type == mu_cfg_node_param &&
	       strcmp (node->tag, "include") == 0)
	{
	  if (node->label->type == MU_CFG_STRING)
	    {
	      mu_cfg_tree_t *t = do_include (node->label->v.string,
					     hints,
					     &node->locus);
	      if (t)
		{
		  /* Merge the new tree into the current point and
		     destroy the rest of it */
		  mu_iterator_ctl (itr, mu_itrctl_insert_list, t->nodes);
		  mu_opool_union (&tree->pool, &t->pool);
		  mu_cfg_destroy_tree (&t);
		}		      
	    }
	  else
	    {
	      mu_diag_at_locus_range (MU_LOG_ERROR, &node->locus,
				_("argument to `include' is not a string"));
	      mu_cfg_error_count++;
	    }
	  
	  /* Remove node from the list */
	  mu_iterator_ctl (itr, mu_itrctl_delete, NULL);
	}
    }
  mu_iterator_destroy (&itr);
  return 0;
}

static int
_mu_cfg_preorder_recursive (void *item, void *cbdata)
{
  mu_cfg_node_t *node = item;
  struct mu_cfg_iter_closure *clos = cbdata;
  int rc;
  
  switch (node->type)
    {
    case mu_cfg_node_undefined:
      abort ();
      
    case mu_cfg_node_statement:
      switch (clos->beg (node, clos->data))
	{
	case MU_CFG_ITER_OK:
	  rc = mu_cfg_preorder (node->nodes, clos);
	  if (rc)
	    return rc;
	  if (clos->end && clos->end (node, clos->data) == MU_CFG_ITER_STOP)
	    return MU_ERR_USER0;
	  break;
	  
	case MU_CFG_ITER_SKIP:
	  break;
	  
	case MU_CFG_ITER_STOP:
	  return MU_ERR_USER0;
	}
      break;
      
    case mu_cfg_node_param:
      if (clos->beg (node, clos->data) == MU_CFG_ITER_STOP)
	return MU_ERR_USER0;
    }
  return 0;
}

int
mu_cfg_preorder (mu_list_t nodelist, struct mu_cfg_iter_closure *clos)
{
  if (!nodelist)
    return 0;
  return mu_list_foreach (nodelist, _mu_cfg_preorder_recursive, clos);
}



void
mu_cfg_destroy_tree (mu_cfg_tree_t **ptree)
{
  if (ptree && *ptree)
    {
      mu_cfg_tree_t *tree = *ptree;
      mu_list_destroy (&tree->nodes);
      mu_opool_destroy (&tree->pool);
      free (tree);
      *ptree = NULL;
    }
}



struct mu_cfg_section_list
{
  struct mu_cfg_section_list *next;
  struct mu_cfg_section *sec;
};

struct scan_tree_data
{
  struct mu_cfg_section_list *list;
  void *target;
  void *call_data;
  mu_cfg_tree_t *tree;
  int error;
};

static struct mu_cfg_cont *
find_container (mu_list_t list, enum mu_cfg_cont_type type,
		const char *ident, size_t len)
{
  mu_iterator_t iter;
  struct mu_cfg_cont *ret = NULL;
  
  if (len == 0)
    len = strlen (ident);
  
  mu_list_get_iterator (list, &iter);
  for (mu_iterator_first (iter); !mu_iterator_is_done (iter);
       mu_iterator_next (iter))
    {
      struct mu_cfg_cont *cont;
      mu_iterator_current (iter, (void**) &cont);
      
      if (cont->type == type
	  && strlen (cont->v.ident) == len
	  && memcmp (cont->v.ident, ident, len) == 0)
	{
	  ret = cont;
	  break;
	}
    }
  mu_iterator_destroy (&iter);
  return ret;
}

static struct mu_cfg_section *
find_subsection (struct mu_cfg_section *sec, const char *ident, size_t len)
{
  if (sec)
    {
      if (sec->children)
	{
	  struct mu_cfg_cont *cont = find_container (sec->children,
						     mu_cfg_cont_section,
						     ident, len);
	  if (cont)
	    return &cont->v.section;
	}
    }
  return NULL;
}

static struct mu_cfg_param *
find_param (struct mu_cfg_section *sec, const char *ident, size_t len)
{
  if (sec)
    {
      if (sec->children)
	{
	  struct mu_cfg_cont *cont = find_container (sec->children,
						     mu_cfg_cont_param,
						     ident, len);
	  if (cont)
	    return &cont->v.param;
	}
    }
  return NULL;
}

static int
push_section (struct scan_tree_data *dat, struct mu_cfg_section *sec)
{
  struct mu_cfg_section_list *p = mu_alloc (sizeof *p);
  if (!p)
    {
      mu_error (_("not enough memory"));
      mu_cfg_error_count++;
      return 1;
    }
  p->sec = sec;
  p->next = dat->list;
  dat->list = p;
  return 0;
}

static struct mu_cfg_section *
pop_section (struct scan_tree_data *dat)
{
  struct mu_cfg_section_list *p = dat->list;
  struct mu_cfg_section *sec = p->sec;
  dat->list = p->next;
  free (p);
  return sec;
}

static int
valcvt (const struct mu_locus_range *locus,
	void *tgt, mu_c_type_t type, mu_config_value_t *val)
{
  int rc;
  char *errmsg;
  
  if (val->type != MU_CFG_STRING)
    {
      mu_diag_at_locus_range (MU_LOG_ERROR, locus, _("expected string value"));
      mu_cfg_error_count++;
      return 1;
    }

  rc = mu_str_to_c (val->v.string, type, tgt, &errmsg);
  if (rc)
    {
      mu_diag_at_locus_range (MU_LOG_ERROR, locus, "%s",
			      errmsg ? errmsg : mu_strerror (rc));
      free (errmsg);
    }
  return rc;
}

struct set_closure
{
  mu_list_t list;
  int type;
  struct scan_tree_data *sdata;
  const struct mu_locus_range *locus;
};

static size_t config_type_size[] = {
  [mu_c_string] = sizeof (char*),          
  [mu_c_short]  = sizeof (short),          
  [mu_c_ushort] = sizeof (unsigned short), 
  [mu_c_int]    = sizeof (int),            
  [mu_c_uint]   = sizeof (unsigned),       
  [mu_c_long]   = sizeof (long),           
  [mu_c_ulong]  = sizeof (unsigned long),  
  [mu_c_size]   = sizeof (size_t),         
  [mu_c_time]   = sizeof (time_t),         
  [mu_c_bool]   = sizeof (int),            
  [mu_c_ipv4]   = sizeof (struct in_addr), 
  [mu_c_cidr]   = sizeof (struct mu_cidr), 
  [mu_c_host]   = sizeof (struct in_addr), 
};

static int
_set_fun (void *item, void *data)
{
  mu_config_value_t *val = item;
  struct set_closure *clos = data;
  void *tgt;
  size_t size;
  
  if ((size_t) clos->type >= MU_ARRAY_SIZE(config_type_size)
      || (size = config_type_size[clos->type]) == 0)
    {
      mu_diag_at_locus_range (MU_LOG_EMERG, clos->locus,
			_("INTERNAL ERROR at %s:%d: unhandled data type %d"),
			__FILE__, __LINE__, clos->type);
      mu_cfg_error_count++;
      return 1;
    }
  
  tgt = mu_alloc (size);
  if (!tgt)
    {
      mu_diag_at_locus_range (MU_LOG_ERROR, clos->locus,
			      _("not enough memory"));
      mu_cfg_error_count++;
      return 1;
    }
  
  if (valcvt (clos->locus, &tgt, clos->type, val) == 0)
    mu_list_append (clos->list, tgt);
  return 0;
}

static int
parse_param (struct scan_tree_data *sdata, const mu_cfg_node_t *node)
{
  void *tgt;
  struct set_closure clos;
  struct mu_cfg_param *param = find_param (sdata->list->sec, node->tag,
					   0);
  
  if (!param)
    {
      mu_diag_at_locus_range (MU_LOG_ERROR, &node->locus,
			      _("unknown keyword `%s'"),
			      node->tag);
      mu_cfg_error_count++;
      return 1;
    }
  
  if (param->data)
    tgt = param->data;
  else if (sdata->list->sec->target)
    tgt = (char*)sdata->list->sec->target + param->offset;
  else if (sdata->target)
    tgt = (char*)sdata->target + param->offset;
  else if (param->type == mu_cfg_callback)
    tgt = NULL;
  else
    {
      mu_diag_at_locus_range (MU_LOG_EMERG, &node->locus,
			_("INTERNAL ERROR: cannot determine target offset for "
			  "%s"), param->ident);
      abort ();
    }
  
  memset (&clos, 0, sizeof clos);
  clos.type = MU_CFG_TYPE (param->type);
  if (MU_CFG_IS_LIST (param->type))
    {
      clos.sdata = sdata;
      clos.locus = &node->locus;
      switch (node->label->type)
	{
	case MU_CFG_LIST:
	  break;
	  
	case MU_CFG_STRING:
	  {
	    mu_list_t list;
	    mu_list_create (&list);
	    mu_list_append (list, config_value_dup (node->label));
	    node->label->type = MU_CFG_LIST;
	    node->label->v.list = list;
	  }
	  break;
	  
	case MU_CFG_ARRAY:
	  mu_diag_at_locus_range (MU_LOG_ERROR, &node->locus,
			    _("expected list, but found array"));
	  mu_cfg_error_count++;
	  return 1;
	}
      
      mu_list_create (&clos.list);
      mu_list_foreach (node->label->v.list, _set_fun, &clos);
      *(mu_list_t*)tgt = clos.list;
    }
  else if (clos.type == mu_cfg_callback)
    {
      if (!param->callback)
	{
	  mu_diag_at_locus_range (MU_LOG_EMERG, &node->locus,
			    _("INTERNAL ERROR: %s: callback not defined"),
			    node->tag);
	  abort ();
	}
      mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, 
		       MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE,
		       (void*) &node->locus);
      if (param->callback (tgt, node->label))
	return 1;
    }
  else
    return valcvt (&node->locus, tgt, clos.type, node->label);
  
  return 0;
}


static int
_scan_tree_helper (const mu_cfg_node_t *node, void *data)
{
  struct scan_tree_data *sdata = data;
  struct mu_cfg_section *sec;
  
  switch (node->type)
    {
    case mu_cfg_node_undefined:
      abort ();
      
    case mu_cfg_node_statement:
      sec = find_subsection (sdata->list->sec, node->tag, 0);
      if (!sec)
	{
	  if (mu_cfg_parser_verbose)
	    {
	      mu_diag_at_locus_range (MU_LOG_WARNING, &node->locus,
				      _("unknown section `%s'"),
				      node->tag);
	    }
	  return MU_CFG_ITER_SKIP;
	}
      if (!sec->children)
	return MU_CFG_ITER_SKIP;
      if (sec->data)
	sec->target = sec->data;
      else if (sdata->list->sec->target)
	sec->target = (char*)sdata->list->sec->target + sec->offset;
      else if (sdata->target)
	sec->target = (char*)sdata->target + sec->offset;
      else
	sec->target = NULL;
      if (sec->parser)
	{
	  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, 
			   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE,
			   (void*) &node->locus);
	  if (sec->parser (mu_cfg_section_start, node,
			   sec->label, &sec->target,
			   sdata->call_data, sdata->tree))
	    {
	      sdata->error++;
	      return MU_CFG_ITER_SKIP;
	    }
	}
      push_section(sdata, sec);
      break;
      
    case mu_cfg_node_param:
      if (parse_param (sdata, node))
	{
	  sdata->error++;
	  return MU_CFG_ITER_SKIP;
	}
      break;
    }
  return MU_CFG_ITER_OK;
}

static int
_scan_tree_end_helper (const mu_cfg_node_t *node, void *data)
{
  struct scan_tree_data *sdata = data;
  struct mu_cfg_section *sec;
  
  switch (node->type)
    {
    default:
      abort ();
      
    case mu_cfg_node_statement:
      sec = pop_section (sdata);
      if (sec && sec->parser)
	{
	  if (sec->parser (mu_cfg_section_end, node,
			   sec->label, &sec->target,
			   sdata->call_data, sdata->tree))
	    {
	      sdata->error++;
	      return MU_CFG_ITER_SKIP;
	    }
	}
    }
  return MU_CFG_ITER_OK;
}

int
mu_cfg_scan_tree (mu_cfg_tree_t *tree, struct mu_cfg_section *sections,
		   void *target, void *data)
{
  struct scan_tree_data dat;
  struct mu_cfg_iter_closure clos;
  int save_mode = 0, mode;
  struct mu_locus_range save_locus = MU_LOCUS_RANGE_INITIALIZER;
  int rc;
  
  dat.tree = tree;
  dat.list = NULL;
  dat.error = 0;
  dat.call_data = data;
  dat.target = target;
  
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, 
		   MU_IOCTL_LOGSTREAM_GET_MODE, &save_mode);
  mode = save_mode | MU_LOGMODE_LOCUS;
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, 
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, 
		   MU_IOCTL_LOGSTREAM_GET_LOCUS_RANGE, &save_locus);

  if (push_section (&dat, sections))
    return 1;
  clos.beg = _scan_tree_helper;
  clos.end = _scan_tree_end_helper;
  clos.data = &dat;
  rc = mu_cfg_preorder (tree->nodes, &clos);
  pop_section (&dat);
  if (rc && rc != MU_ERR_USER0)
    dat.error++;
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, 
		   MU_IOCTL_LOGSTREAM_SET_MODE, &save_mode);
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, 
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &save_locus);
  
  return dat.error;
}

int
mu_cfg_find_section (struct mu_cfg_section *root_sec,
		     const char *path, struct mu_cfg_section **retval)
{
  while (path[0])
    {
      struct mu_cfg_section *sec;
      size_t len;
      const char *p;
      
      while (*path == MU_CFG_PATH_DELIM)
	path++;
      
      if (*path == 0)
	return MU_ERR_NOENT;
      
      p = strchr (path, MU_CFG_PATH_DELIM);
      if (p)
	len = p - path;
      else
	len = strlen (path);
      
      sec = find_subsection (root_sec, path, len);
      if (!sec)
	return MU_ERR_NOENT;
      root_sec = sec;
      path += len;
    }
  if (retval)
    *retval = root_sec;
  return 0;
}


int
mu_cfg_tree_create (struct mu_cfg_tree **ptree)
{
  struct mu_cfg_tree *tree = calloc (1, sizeof *tree);
  if (!tree)
    return errno;
  mu_opool_create (&tree->pool, MU_OPOOL_ENOMEMABRT);
  *ptree = tree;
  return 0;
}

mu_cfg_node_t *
mu_cfg_tree_create_node (struct mu_cfg_tree *tree,
			 enum mu_cfg_node_type type,
			 const struct mu_locus_range *loc,
			 const char *tag, const char *label,
			 mu_list_t nodelist)
{
  char *p;
  mu_cfg_node_t *np;
  size_t size = sizeof *np + strlen (tag) + 1;
  mu_config_value_t val;
  
  np = mu_alloc (size);
  np->type = type;
  mu_locus_range_init (&np->locus);
  if (loc)
    mu_locus_range_copy (&np->locus, loc);
  p = (char*) (np + 1);
  np->tag = p;
  strcpy (p, tag);
  p += strlen (p) + 1;
  val.type = MU_CFG_STRING;
  if (label)
    {
      mu_opool_clear (tree->pool);
      mu_opool_appendz (tree->pool, label);
      mu_opool_append_char (tree->pool, 0);
      val.v.string = mu_opool_finish (tree->pool, NULL);
      np->label = config_value_dup (&val);
    }
  else
    np->label = NULL;
  np->nodes = nodelist;
  return np;
}

void
mu_cfg_tree_add_node (mu_cfg_tree_t *tree, mu_cfg_node_t *node)
{
  if (!node)
    return;
  if (!tree->nodes)
    /* FIXME: return code? */
    mu_cfg_create_node_list (&tree->nodes);
  mu_list_append (tree->nodes, node);
}

void
mu_cfg_tree_add_nodelist (mu_cfg_tree_t *tree, mu_list_t nodelist)
{
  if (!nodelist)
    return;
  if (!tree->nodes)
    /* FIXME: return code? */
    mu_cfg_create_node_list (&tree->nodes);
  mu_list_append_list (tree->nodes, nodelist);
}


/* Return 1 if configuration value A equals B */
int
mu_cfg_value_eq (mu_config_value_t *a, mu_config_value_t *b)
{
  if (a->type != b->type)
    return 0;
  switch (a->type)
    {
    case MU_CFG_STRING:
      if (a->v.string == NULL)
	return b->v.string == NULL;
      return strcmp (a->v.string, b->v.string) == 0;
      
    case MU_CFG_LIST:
      {
	int ret = 1;
	size_t cnt;
	size_t i;
	mu_iterator_t aitr, bitr;
	
	mu_list_count (a->v.list, &cnt);
	mu_list_count (b->v.list, &i);
	if (i != cnt)
	  return 1;
	
	mu_list_get_iterator (a->v.list, &aitr);
	mu_list_get_iterator (b->v.list, &bitr);
	for (i = 0,
	       mu_iterator_first (aitr),
	       mu_iterator_first (bitr);
	     !mu_iterator_is_done (aitr) && !mu_iterator_is_done (bitr);
	     mu_iterator_next (aitr),
	       mu_iterator_next (bitr),
	       i++)
	  {
	    mu_config_value_t *ap, *bp;
	    mu_iterator_current (aitr, (void**)&ap);
	    mu_iterator_current (bitr, (void**)&bp);
	    ret = mu_cfg_value_eq (ap, bp);
	    if (!ret)
	      break;
	  }
	mu_iterator_destroy (&aitr);
	mu_iterator_destroy (&bitr);
	return ret && i == cnt;
      }
      
    case MU_CFG_ARRAY:
      if (a->v.arg.c == b->v.arg.c)
	{
	  size_t i;
	  for (i = 0; i < a->v.arg.c; i++)
	    if (!mu_cfg_value_eq (&a->v.arg.v[i], &b->v.arg.v[i]))
	      return 0;
	  return 1;
	}
    }
  return 0;
}


static int
split_cfg_path (const char *path, int *pargc, char ***pargv)
{
  int argc;
  char **argv;
  char *delim = MU_CFG_PATH_DELIM_STR;
  char static_delim[2] = { 0, 0 };
  
  if (path[0] == '\\')
    {
      argv = calloc (2, sizeof (*argv));
      if (!argv)
	return ENOMEM;
      argv[0] = strdup (path + 1);
      if (!argv[0])
	{
	  free (argv);
	  return ENOMEM;
	}
      argv[1] = NULL;
      argc = 1;
    }
  else
    {
      struct mu_wordsplit ws;
      
      if (mu_ispunct (path[0]))
	{
	  delim = static_delim;
	  delim[0] = path[0];
	  path++;
	}
      ws.ws_delim = delim;
      
      if (mu_wordsplit (path, &ws, MU_WRDSF_DEFFLAGS|MU_WRDSF_DELIM))
	{
	  mu_error (_("cannot split line `%s': %s"), path,
		    mu_wordsplit_strerror (&ws));
	  mu_wordsplit_free (&ws);
	  return errno;
	}
      argc = ws.ws_wordc;
      argv = ws.ws_wordv;
      ws.ws_wordc = 0;
      ws.ws_wordv = NULL;
      mu_wordsplit_free (&ws);
    }
  
  *pargc = argc;
  *pargv = argv;
  
  return 0;
}

struct find_data
{
  int argc;
  char **argv;
  int tag;
  mu_config_value_t *label;
  const mu_cfg_node_t *node;
};

static void
free_value_mem (mu_config_value_t *p)
{
  switch (p->type)
    {
    case MU_CFG_STRING:
      free ((char*)p->v.string);
      break;
      
    case MU_CFG_LIST:
      /* FIXME */
      break;
      
    case MU_CFG_ARRAY:
      {
	size_t i;
	for (i = 0; i < p->v.arg.c; i++)
	  free_value_mem (&p->v.arg.v[i]);
      }
    }
}

static void
destroy_value (void *p)
{
  mu_config_value_t *val = p;
  if (val)
    {
      free_value_mem (val);
      free (val);
    }
}

static mu_config_value_t *
parse_label (const char *str)
{
  mu_config_value_t *val = NULL;
  size_t i;
  struct mu_wordsplit ws;
  size_t len = strlen (str);
  
  if (len > 1 && str[0] == '(' && str[len-1] == ')')
    {
      mu_list_t lst;
      
      ws.ws_delim = ",";
      if (mu_wordsplit_len (str + 1, len - 2, &ws,
			    MU_WRDSF_DEFFLAGS|MU_WRDSF_DELIM|MU_WRDSF_WS))
	{
	  mu_error (_("cannot split line `%s': %s"), str,
		    mu_wordsplit_strerror (&ws));
	  return NULL;
	}
      
      mu_list_create (&lst);
      mu_list_set_destroy_item (lst, destroy_value);
      for (i = 0; i < ws.ws_wordc; i++)
	{
	  mu_config_value_t *p = mu_alloc (sizeof (*p));
	  p->type = MU_CFG_STRING;
	  p->v.string = ws.ws_wordv[i];
	  mu_list_append (lst, p);
	}
      val = mu_alloc (sizeof (*val));
      val->type = MU_CFG_LIST;
      val->v.list = lst;
    }
  else
    {      
      if (mu_wordsplit (str, &ws, MU_WRDSF_DEFFLAGS))
	{
	  mu_error (_("cannot split line `%s': %s"), str,
		    mu_wordsplit_strerror (&ws));
	  return NULL;
	}
      val = mu_alloc (sizeof (*val));
      if (ws.ws_wordc == 1)
	{
	  val->type = MU_CFG_STRING;
	  val->v.string = ws.ws_wordv[0];
	}
      else
	{
	  val->type = MU_CFG_ARRAY;
	  val->v.arg.c = ws.ws_wordc;
	  val->v.arg.v = mu_alloc (ws.ws_wordc * sizeof (val->v.arg.v[0]));
	  for (i = 0; i < ws.ws_wordc; i++)
	    {
	      val->v.arg.v[i].type = MU_CFG_STRING;
	      val->v.arg.v[i].v.string = ws.ws_wordv[i];
	    }
	}
      ws.ws_wordc = 0;
      mu_wordsplit_free (&ws);
    }
  return val;
}

static void
parse_tag (struct find_data *fptr)
{
  char *p = strchr (fptr->argv[fptr->tag], '=');
  if (p)
    {
      *p++ = 0;
      fptr->label = parse_label (p);
    }
  else
    fptr->label = NULL;
}

static int
node_finder (const mu_cfg_node_t *node, void *data)
{
  struct find_data *fdptr = data;
  if (strcmp (fdptr->argv[fdptr->tag], node->tag) == 0
      && (!fdptr->label || mu_cfg_value_eq (fdptr->label, node->label)))
    {
      fdptr->tag++;
      if (fdptr->tag == fdptr->argc)
	{
	  fdptr->node = node;
	  return MU_CFG_ITER_STOP;
	}
      parse_tag (fdptr);
      return MU_CFG_ITER_OK;
    }
  
  return node->type == mu_cfg_node_statement ?
               MU_CFG_ITER_SKIP : MU_CFG_ITER_OK;
}

int	    
mu_cfg_find_node (mu_cfg_tree_t *tree, const char *path, mu_cfg_node_t **pval)
{
  int rc;
  struct find_data data;
  
  rc = split_cfg_path (path, &data.argc, &data.argv);
  if (rc)
    return rc;
  data.tag = 0;
  if (data.argc)
    {
      struct mu_cfg_iter_closure clos;
      
      parse_tag (&data);
      
      clos.beg = node_finder;
      clos.end = NULL;
      clos.data = &data;
      rc = mu_cfg_preorder (tree->nodes, &clos);
      destroy_value (data.label);
      if (rc == MU_ERR_USER0)
	{
	  *pval = (mu_cfg_node_t *) data.node;
	  return 0;
	}
      else if (rc != 0)
	mu_diag_funcall (MU_DIAG_ERR, "mu_cfg_preorder", NULL, rc);
    }
  return MU_ERR_NOENT;
}



int
mu_cfg_create_subtree (const char *path, mu_cfg_node_t **pnode)
{
  int rc;
  int argc, i;
  char **argv;
  enum mu_cfg_node_type type;
  mu_cfg_node_t *node = NULL;
  struct mu_locus_range locus = MU_LOCUS_RANGE_INITIALIZER;
  
  rc = split_cfg_path (path, &argc, &argv);
  if (rc)
    return rc;
  
  for (i = argc - 1; i >= 0; i--)
    {
      mu_list_t nodelist = NULL;
      mu_config_value_t *label = NULL;
      char *q = argv[i], *p;
      mu_cfg_node_t *parent;
      
      type = mu_cfg_node_statement;
      do
	{
	  p = strchr (q, '=');
	  if (p && p > argv[i] && p[-1] != '\\')
	    {
	      *p++ = 0;
	      label = parse_label (p);
	      if (i == argc - 1)
		type = mu_cfg_node_param;
	      break;
	    }
	  else if (p)
	    q = p + 1;
	  else
	    break;
	}
      while (*q);
      
      if (node)
	{
	  mu_cfg_create_node_list (&nodelist);
	  mu_list_append (nodelist, node);
	}
      parent = mu_cfg_alloc_node (type, &locus, argv[i], label, nodelist);
      if (node)
	node->parent = parent;
      node = parent;
    }
  
  mu_argcv_free (argc, argv);
  *pnode = node;
  return 0;
}

int
mu_cfg_parse_config (mu_cfg_tree_t **ptree, struct mu_cfg_parse_hints *hints)
{
  int rc = 0;
  mu_cfg_tree_t *tree = NULL, *tmp;
  struct mu_cfg_parse_hints xhints;
  
  if ((hints->flags & MU_CFHINT_SITE_FILE) && hints->site_file)
    {
      rc = mu_cfg_parse_file (&tmp, hints->site_file, hints->flags);
      
      switch (rc)
	{
	case 0:
	  mu_cfg_tree_postprocess (tmp, hints);
	  mu_cfg_tree_union (&tree, &tmp);
	  
	case ENOENT:
	  rc = 0;
	  break;

	default:
	  mu_error ("%s", mu_strerror (rc));
	  return rc;
	}
    }

  xhints = *hints;
  xhints.flags &= ~MU_CFHINT_PROGRAM;
  
  if ((hints->flags & MU_CFHINT_PER_USER_FILE)
      && (hints->flags & MU_CFHINT_PROGRAM))
    {
      size_t size = 3 + strlen (hints->program) + 1;
      char *file_name = malloc (size);
      if (file_name)
	{
	  strcpy (file_name, "~/.");
	  strcat (file_name, hints->program);
	  
	  rc = mu_cfg_parse_file (&tmp, file_name, xhints.flags);
	  switch (rc)
	    {
	    case 0:
	      mu_cfg_tree_postprocess (tmp, &xhints);
	      mu_cfg_tree_union (&tree, &tmp);
	      break;

	    case ENOENT:
	      rc = 0;
	      break;
	      
	    default:
	      mu_error ("%s", mu_strerror (rc));
	      mu_cfg_destroy_tree (&tree);
	      return rc;
	    }
	  free (file_name);
	}
    }
  
  if ((hints->flags & MU_CFHINT_CUSTOM_FILE) && hints->custom_file)
    {
      rc = mu_cfg_parse_file (&tmp, hints->custom_file, xhints.flags);
      if (rc)
	{
	  mu_error (_("errors parsing file %s: %s"), hints->custom_file,
		    mu_strerror (rc));
	  mu_cfg_destroy_tree (&tree);
	  return rc;
	}
      else
	{
	  mu_cfg_tree_postprocess (tmp, &xhints);
	  mu_cfg_tree_union (&tree, &tmp);
	}
    }

  *ptree = tree;
  return rc;
}

  
	  
        
  
