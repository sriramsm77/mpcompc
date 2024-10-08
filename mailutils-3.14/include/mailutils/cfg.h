/* cfg.h -- general-purpose configuration file parser 
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

#ifndef _MAILUTILS_CFG_H
#define _MAILUTILS_CFG_H

#include <mailutils/types.h>
#include <mailutils/list.h>
#include <mailutils/debug.h>
#include <mailutils/opool.h>
#include <mailutils/util.h>
#include <mailutils/locus.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif  

typedef struct mu_cfg_node mu_cfg_node_t;
typedef struct mu_cfg_tree mu_cfg_tree_t;

#define MU_CFG_STRING 0
#define MU_CFG_LIST   1
#define MU_CFG_ARRAY  2

typedef struct mu_config_value mu_config_value_t;

struct mu_config_value   
{
  int type;
  union
  {
    mu_list_t list;
    const char *string;
    struct
    {
      size_t c;
      mu_config_value_t *v;
    } arg;
  } v;
};
  
enum mu_cfg_node_type
  {
    mu_cfg_node_undefined,
    mu_cfg_node_statement,
    mu_cfg_node_param
  };

struct mu_cfg_node
{
  struct mu_locus_range locus;
  enum mu_cfg_node_type type;
  char *tag;
  mu_config_value_t *label;
  mu_list_t nodes;   /* a list of mu_cfg_node_t */
  struct mu_cfg_node *parent; /* parent node */
};

struct mu_cfg_parse_hints
{
  int flags;
  char *site_file;
  char *custom_file;
  char *program;
};

/* Bit constants for the flags field of struct mu_cfg_parse_hints */
/* Parse site-wide configuration file hints.site_file */
#define MU_CFHINT_SITE_FILE          0x0001
/* Parse custom configuration file hints.custom_file */
#define MU_CFHINT_CUSTOM_FILE        0x0002
/* The hints.program field is set.  The "program PROGNAME" section
   will be processed, if PROGNAME is the same as hints.program.
   If include statement is used with the directory name DIR as its
   argument, the file DIR/PROGNAME will be looked up and read in,
   if it exists. */
#define MU_CFHINT_PROGRAM            0x0004

/* If MU_CFHINT_PROGRAM is set, look for the file ~/.PROGNAME after parsing
   site-wide configuration */
#define MU_CFHINT_PER_USER_FILE      0x0008

/* Don't allow to override configuration settings from the command line. */
#define MU_CFHINT_NO_CONFIG_OVERRIDE 0x0010
	
/* Verbosely log files being processed */
#define MU_CF_VERBOSE                0x0100
/* Dump the pare tree on stderr */
#define MU_CF_DUMP                   0x0200

/* Format location of the statement */
#define MU_CF_FMT_LOCUS              0x1000
/* Print only value */
#define MU_CF_FMT_VALUE_ONLY         0x2000
/* Print full parameter path */
#define MU_CF_FMT_PARAM_PATH         0x4000
  
struct mu_cfg_tree
{
  mu_list_t nodes;   /* a list of mu_cfg_node_t */
  mu_opool_t pool;
};

int mu_cfg_parse (mu_cfg_tree_t **ptree);
int mu_cfg_tree_union (mu_cfg_tree_t **pa, mu_cfg_tree_t **pb);
int mu_cfg_tree_postprocess (mu_cfg_tree_t *tree,
			     struct mu_cfg_parse_hints *hints);

mu_opool_t mu_cfg_lexer_pool (void);

#define MU_CFG_ITER_OK   0
#define MU_CFG_ITER_SKIP 1
#define MU_CFG_ITER_STOP 2

typedef int (*mu_cfg_iter_func_t) (const mu_cfg_node_t *node, void *data);

struct mu_cfg_iter_closure
{
  mu_cfg_iter_func_t beg;
  mu_cfg_iter_func_t end;
  void *data;
};

void mu_cfg_destroy_tree (mu_cfg_tree_t **tree);

int mu_cfg_preorder (mu_list_t nodelist, struct mu_cfg_iter_closure *);


#define MU_CFG_LIST_MASK 0x8000
#define MU_CFG_LIST_OF(t) ((t) | MU_CFG_LIST_MASK)
#define MU_CFG_TYPE(t) ((t) & ~MU_CFG_LIST_MASK)
#define MU_CFG_IS_LIST(t) ((t) & MU_CFG_LIST_MASK)
  
typedef int (*mu_cfg_callback_t) (void *, mu_config_value_t *);

enum mu_cfg_param_type
  {
    mu_cfg_section = mu_c_void + 1,
    mu_cfg_callback
  };
  
struct mu_cfg_param
{
  const char *ident;
  int type;       /* One of enum mu_c_type or mu_cfg_param_type values */
  void *data;
  size_t offset;
  mu_cfg_callback_t callback;
  const char *docstring;
  const char *argname;
};

enum mu_cfg_section_stage
  {
    mu_cfg_section_start,
    mu_cfg_section_end
  };

typedef int (*mu_cfg_section_fp) (enum mu_cfg_section_stage stage,
				  const mu_cfg_node_t *node,
				  const char *label,
				  void **section_data_ptr,
				  void *call_data,
				  mu_cfg_tree_t *tree);

struct mu_cfg_section
{
  const char *ident;              /* Section identifier */  
  char *label;                    /* Label description */
  mu_cfg_section_fp parser;       /* Parser function */
  void *data;                     /* Data pointer */
  size_t offset;                  /* Offset within target (see below) */
  mu_list_t /* of mu_cfg_cont */ children;
  char *docstring;                /* Documentation string */
  void *target;                   /* Actual pointer to the data. It is
				     recomputed each time the section is
				     reduced. */
  char *ident_storage;            /* Storage for ident, if malloc'ed */
  char *label_storage;            /* Same for the label */
};

enum mu_cfg_cont_type
  {
    mu_cfg_cont_section,
    mu_cfg_cont_param
  };

struct mu_cfg_cont
{
  enum mu_cfg_cont_type type;
  mu_refcount_t refcount;
  union
  {
    const char *ident;
    struct mu_cfg_section section;
    struct mu_cfg_param param;
  } v;
};

#define MU_CFG_PATH_DELIM '.'
#define MU_CFG_PATH_DELIM_STR "."
  
int mu_config_create_container (struct mu_cfg_cont **pcont,
				enum mu_cfg_cont_type type);
int mu_config_clone_container (struct mu_cfg_cont *cont);
struct mu_cfg_cont *mu_config_clone_root_container (void);

void mu_config_destroy_container (struct mu_cfg_cont **pcont);

int mu_cfg_section_add_container (struct mu_cfg_section *sect,
				  struct mu_cfg_cont *cont);
int mu_cfg_section_add_params (struct mu_cfg_section *sect,
			       struct mu_cfg_param *param);


int mu_create_canned_section (char *name, struct mu_cfg_section **psection);
int mu_create_canned_param (char *name, struct mu_cfg_param **pparam);
struct mu_cfg_cont *mu_get_canned_container (const char *name);

int mu_cfg_create_node_list (mu_list_t *plist);
  
int mu_cfg_scan_tree (mu_cfg_tree_t *tree, struct mu_cfg_section *sections,
		      void *target, void *call_data);

int mu_cfg_find_section (struct mu_cfg_section *root_sec,
			 const char *path, struct mu_cfg_section **retval);

int mu_config_container_register_section (struct mu_cfg_cont **proot,
					  const char *parent_path,
					  const char *ident,
					  const char *label,
					  mu_cfg_section_fp parser,
					  struct mu_cfg_param *param,
					  struct mu_cfg_section **psection);
int mu_config_root_register_section (const char *parent_path,
				     const char *ident,
				     const char *label,
				     mu_cfg_section_fp parser,
				     struct mu_cfg_param *param);

int mu_config_register_plain_section (const char *parent_path,
				      const char *ident,
				      struct mu_cfg_param *params);

  
extern int mu_cfg_parser_verbose;
extern size_t mu_cfg_error_count;

void mu_cfg_format_docstring (mu_stream_t stream, const char *docstring,
			      int level);
void mu_cfg_format_parse_tree (mu_stream_t stream, struct mu_cfg_tree *tree,
			       int flags);
void mu_cfg_format_node (mu_stream_t stream, const mu_cfg_node_t *node,
			 int flags);
  
void mu_cfg_format_container (mu_stream_t stream, struct mu_cfg_cont *cont);
void mu_format_config_tree (mu_stream_t stream,
			    struct mu_cfg_param *progparam);
int mu_cfg_tree_reduce (mu_cfg_tree_t *parse_tree,
                        struct mu_cfg_parse_hints *hints,
		        struct mu_cfg_param *progparam,
		        void *target_ptr);
int mu_cfg_assert_value_type (mu_config_value_t *val, int type);
int mu_cfg_string_value_cb (mu_config_value_t *val,
			    int (*fun) (const char *, void *),
			    void *data);

int mu_cfg_parse_file (mu_cfg_tree_t **return_tree, const char *file,
		       int flags);
  
  
int mu_cfg_tree_create (struct mu_cfg_tree **ptree);
mu_cfg_node_t *mu_cfg_tree_create_node (struct mu_cfg_tree *tree,
					enum mu_cfg_node_type type,
					const struct mu_locus_range *loc,
					const char *tag,
					const char *label,
					mu_list_t nodelist);
void mu_cfg_tree_add_node (mu_cfg_tree_t *tree, mu_cfg_node_t *node);
void mu_cfg_tree_add_nodelist (mu_cfg_tree_t *tree, mu_list_t nodelist);

int mu_cfg_find_node (mu_cfg_tree_t *tree, const char *path,
		      mu_cfg_node_t **pnode);
int mu_cfg_create_subtree (const char *path, mu_cfg_node_t **pnode);

int mu_cfg_parse_config (mu_cfg_tree_t **ptree,
			 struct mu_cfg_parse_hints *hints);

int mu_cfg_field_map (struct mu_config_value const *val, mu_assoc_t *passoc,
		      char **err_term);


#ifdef __cplusplus
}
#endif

#endif
