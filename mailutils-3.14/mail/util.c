/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2022 Free Software Foundation, Inc.

   GNU Mailutils is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Mailutils is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>. */

#include "mail.h"
#include <mailutils/util.h>
#include <mailutils/mime.h>
#include <mailutils/folder.h>
#include <mailutils/auth.h>
#include <pwd.h>
#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif
#include <sys/ioctl.h>
#include <sys/stat.h>

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#else
# include <sys/fcntl.h>
#endif
#include <mailutils/io.h>

int
util_do_command (const char *fmt, ...)
{
  struct mu_wordsplit ws;
  int argc;
  char **argv;
  int status = 0;
  const struct mail_command_entry *entry = NULL;
  char *cmd = NULL;
  size_t size = 0;
  va_list ap;

  va_start (ap, fmt);
  status = mu_vasnprintf (&cmd, &size, fmt, ap);
  va_end (ap);
  if (status)
    return status;

  if (cmd)
    {
      /*  Ignore comments */
      if (cmd[0] == '#')
	{
	  free (cmd);
	  return 0;
	}

      if (cmd[0] == '\0')
	{
	  free (cmd);

	  /* Hitting return i.e. no command, is equivalent to next
	     according to the POSIX spec. Note, that this applies
	     to interactive state only. */
	  if (interactive)
	    cmd = mu_strdup ("next");
	  else
	    return 0;
	}

      ws.ws_offs = 1; /* Keep one extra slot in case we need it
			 for expansion (see below) */
      if (mu_wordsplit (cmd, &ws, MU_WRDSF_DEFFLAGS|MU_WRDSF_DOOFFS))
	{
	  mu_error ("\"%s\": %s", cmd, mu_wordsplit_strerror (&ws));
	  free (cmd);
	  return MU_ERR_PARSE;
	}
      else
	{
	  char *p;

	  argc = ws.ws_wordc;
	  argv = ws.ws_wordv + 1;

	  /* Special case: a number alone implies "print" */
	  if (argc == 1
	      && ((strtoul (argv[0], &p, 10) > 0 && *p == 0)
		  || (argv[0][1] == 0 && strchr ("^$", argv[0][0]))))
	    {
	      /* Use the extra slot for "print" command */
	      argc++;
	      argv--;
	      argv[0] = "print";
	    }

	  entry = mail_find_command (argv[0]);
	  free (cmd);
	}
    }
  else
    entry = mail_find_command (mailvar_name_quit);

  if (!entry)
    {
      /* argv[0] might be a traditional /bin/mail contracted form, e.g.
	 `d*' or `p4'. */

      char *p;

      for (p = argv[0] + strlen (argv[0]) - 1;
	   p > argv[0] && !mu_isalpha (*p);
	   p--)
	;

      p++;

      if (strlen (p))
	{
	  /* Expand contracted form.  That's what we have kept an extra
	     ws slot for. */
	  argc++;
	  argv--;
	  argv[0] = argv[1];
	  argv[1] = mu_strdup (p);
	  *p = 0;
	  /* Register the new entry in WS */
	  ws.ws_wordc++;
	  ws.ws_offs = 0;
	}

      entry = mail_find_command (argv[0]);
    }

  if (entry)
    {
      /* Make sure we are not in any if/else */
      if (!(if_cond () == 0 && (entry->flags & EF_FLOW) == 0))
	status = entry->func (argc, argv);
    }
  else
    {
      if (argc)
	mu_error (_("Unknown command: %s"), argv[0]);
      else
	mu_error (_("Invalid command"));
      status = 1;
    }

  mu_wordsplit_free (&ws);
  return status;
}

int
util_foreach_msg (int argc, char **argv, int flags,
		  msg_handler_t func, void *data)
{
  msgset_t *list = NULL, *mp;
  int status = 0;

  if (msgset_parse (argc, argv, flags, &list))
      return 1;

  for (mp = list; mp; mp = mp->next)
    {
      mu_message_t mesg;

      if (util_get_message (mbox, msgset_msgno (mp), &mesg) == 0)
	{
	  if (func (mp, mesg, data) != 0)
	    status = 1;
	  /* Bail out if we receive an interrupt.  */
	  if (ml_got_interrupt () != 0)
	    break;
	}
    }
  msgset_free (list);

  return status;
}

size_t
util_range_msg (size_t low, size_t high, int flags,
		msg_handler_t func, void *data)
{
  size_t count, expect_count;

  if (!func)
    flags |= MSG_SILENT;

  if (low > total)
    return 0;
  if (!(flags & MSG_COUNT))
    {
      if (high < low)
	return 0;
      expect_count = high - low + 1;
    }
  else
    expect_count = high;

  for (count = 0; count < expect_count && low <= total; low++)
    {
     mu_message_t mesg;

     if ((flags & MSG_NODELETED) && util_isdeleted (low))
       {
	 if (!(flags & MSG_SILENT))
	   mu_error (_("%lu: Inappropriate message (has been deleted)"),
		       (unsigned long) low);
	 continue;
       }

     if (util_get_message (mbox, low, &mesg) == 0)
       {
	 count ++;
	 if (func)
	   {
	     msgset_t *set = msgset_make_1 (low);
	     func (set, mesg, data);
	     msgset_free (set);
	   }
	 /* Bail out if we receive an interrupt.  */
	 if (ml_got_interrupt () != 0)
	   break;
       }
    }
  return count;
}

/*
 * returns the function to run for command
 */
function_t *
util_command_get (const char *cmd)
{
  const struct mail_command_entry *entry = mail_find_command (cmd);
  return entry ? entry->func : NULL;
}

/*
 * returns the mail_command_entry structure for the command matching cmd
 */
void *
util_find_entry (void *table, size_t nmemb, size_t size, const char *cmd)
{
  int i;
  int len = strlen (cmd);
  char *p;

  for (p = table, i = 0; i < nmemb; i++, p += size)
    {
      struct mail_command *cp = (struct mail_command *)p;
      int ll = strlen (cp->longname);
      int sl = strlen (cp->shortname);

      if (sl > ll && !strncmp (cp->shortname, cmd, sl))
	return p;
      else if (sl == len && !strcmp (cp->shortname, cmd))
	return p;
      else if (sl < len && !strncmp (cp->longname, cmd, len))
	return p;
    }
  return NULL;
}

int
util_help (void *table, size_t nmemb, size_t size, const char *word)
{
  if (!word)
    {
      int i = 0;
      mu_stream_t out;
      char *p;

      out = open_pager (util_screen_lines () + 1);

      for (p = table, i = 0; i < nmemb; i++, p += size)
	{
	  struct mail_command *cp = (struct mail_command *)p;
	  if (cp->synopsis == NULL)
	    continue;
	  mu_stream_printf (out, "%s\n", cp->synopsis);
	}

      mu_stream_unref (out);

      return 0;
    }
  else
    {
      int status = 0;
      struct mail_command *cp = util_find_entry (table, nmemb, size, word);
      if (cp && cp->synopsis)
	mu_printf ("%s\n", cp->synopsis);
      else
	{
	  status = 1;
	  mu_printf (_("Unknown command: %s\n"), word);
	}
      return status;
    }
  return 1;
}

int
util_command_list (void *table, size_t nmemb, size_t size)
{
  int i;
  char *p;
  int cols = util_screen_columns ();
  int pos = 0;

  for (p = table, i = 0; i < nmemb; i++, p += size)
    {
      const char *cmd;
      struct mail_command *cp = (struct mail_command *)p;
      int len = strlen (cp->longname);
      if (len < 1)
	{
	  cmd = cp->shortname;
	  len = strlen (cmd);
	}
      else
	cmd = cp->longname;

      pos += len + 1;

      if (pos >= cols)
	{
	  pos = len + 1;
	  mu_printf ("\n%s ", cmd);
	}
      else
	mu_printf ("%s ", cmd);
    }
  mu_printf ("\n");
  return 0;
}

/*
 * Get the number of columns on the screen
 * First try an ioctl() call not all shells set the COLUMNS environ.
 */
int
util_getcols (void)
{
  struct winsize ws;
  ws.ws_col = ws.ws_row = 0;
  if ((ioctl(1, TIOCGWINSZ, (char *) &ws) < 0) || ws.ws_col == 0)
    return 80;  /* FIXME: Should we exit()/abort() if col <= 0 ?  */
  return ws.ws_col;
}

/*
 * Get the number of lines on the screen
 * First try an ioctl() call not all shells set the LINES environ.
 */
int
util_getlines (void)
{
  struct winsize ws;
  ws.ws_col = ws.ws_row = 0;
  if ((ioctl(1, TIOCGWINSZ, (char *) &ws) < 0) || ws.ws_row <= 2)
    ws.ws_row = 24;  /* FIXME: Should we exit()/abort() if row <= 2 ?  */
  /* Reserve at least 2 line for the prompt.  */
  return ws.ws_row - 2;
}

int
util_screen_lines ()
{
  int screen;
  size_t n;

  if (mailvar_get (&screen, mailvar_name_screen, mailvar_type_number, 0) == 0)
    return screen;
  n = util_getlines();
  util_do_command ("set screen=%lu", (unsigned long) n);
  return n;
}

int
util_screen_columns ()
{
  int cols;
  size_t n;

  if (mailvar_get (&cols, mailvar_name_columns, mailvar_type_number, 0) == 0)
    return cols;
  n = util_getcols();
  util_do_command ("set columns=%lu", (unsigned long) n);
  return n;
}

int
util_get_crt ()
{
  int lines;

  if (mailvar_get (&lines, mailvar_name_crt, mailvar_type_number, 0) == 0)
    return lines;
  else if (mailvar_is_true (mailvar_name_crt))
    return util_getlines ();
  return 0;
}

const char *
util_reply_prefix ()
{
  char *prefix = "Re: ";
  mailvar_get (&prefix, mailvar_name_replyprefix, mailvar_type_string, 0);
  return prefix;
}


/* ************************* */

/*
 * return 1 if a message is deleted
 */

int
util_isdeleted (size_t n)
{
  mu_message_t msg = NULL;
  mu_attribute_t attr = NULL;

  mu_mailbox_get_message (mbox, n, &msg);
  mu_message_get_attribute (msg, &attr);
  return mu_attribute_is_deleted (attr);
}

void
util_mark_read (mu_message_t msg)
{
  mu_attribute_t attr;

  mu_message_get_attribute (msg, &attr);
  mu_attribute_set_read (attr);
  mu_attribute_set_userflag (attr, MAIL_ATTRIBUTE_SHOWN);
}

char *
util_get_homedir ()
{
  char *homedir = mu_get_homedir ();
  if (!homedir)
    {
      /* Shouldn't happen, but one never knows */
      mu_error (_("Cannot get homedir"));
      exit (EXIT_FAILURE);
    }
  return homedir;
}

char *
util_fullpath (const char *inpath)
{
  return mu_tilde_expansion (inpath, MU_HIERARCHY_DELIMITER, NULL);
}

char *
util_folder_path (const char *name)
{
  char *folder;
  int rc;

  if (mailvar_get (&folder, mailvar_name_folder, mailvar_type_string, 1))
    return NULL;

  if (!name)
    return NULL;

  rc = mu_mailbox_expand_name (name, &folder);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mailbox_expand_name", name, rc);
      return NULL;
    }
  return folder;
}

int outfilename_mode;

char *
util_outfilename (mu_address_t addr)
{
  char *buf, *p;
  int rc;
  
  if ((rc = mu_address_aget_email (addr, 1, &buf)) != 0)
    {
      mu_error (_("Cannot determine sender name: %s"), mu_strerror (rc));
      return NULL;
    }

  switch (mailvar_is_true (mailvar_name_mailx)
	  ? outfilename_local : outfilename_mode)
    {
    case outfilename_local:
      p = strchr (buf, '@');
      if (p)
	*p = 0;
      break;

    case outfilename_email:
      break;

    case outfilename_domain:
      p = strchr (buf, '@');
      if (p)
	{
	  p++;
	  memmove (buf, p, strlen (p) + 1);
	}
      else
	{
	  free (buf);
	  buf = mu_strdup ("localdomain");
	}
      break;
    }
  return buf;
}

char *
util_message_sender (mu_message_t msg, int strip)
{
  mu_address_t addr = get_sender_address (msg);
  char *buf;
  int rc;
  
  if (!addr)
    {
      mu_envelope_t env = NULL;
      const char *buffer;
      mu_message_get_envelope (msg, &env);
      if (mu_envelope_sget_sender (env, &buffer)
	  || mu_address_create (&addr, buffer))
	{
	  mu_error (_("Cannot determine sender name"));
	  return NULL;
	}
    }

  if (strip)
    {
      buf = util_outfilename (addr);
    }
  else if ((rc = mu_address_aget_printable (addr, &buf)) != 0)
    {
      mu_error (_("Cannot determine sender name: %s"), mu_strerror (rc));
      buf = NULL;
    }
    
  mu_address_destroy (&addr);
  return buf;
}

char *
util_get_sender (int msgno, int strip)
{
  mu_message_t msg;
  if (mu_mailbox_get_message (mbox, msgno, &msg) == 0)
    {
      return util_message_sender (msg, strip);
    }
  return 0;
}

void
util_slist_print (mu_list_t list, int nl)
{
  mu_iterator_t itr;
  char *name;

  if (!list || mu_list_get_iterator (list, &itr))
    return;

  for (mu_iterator_first (itr); !mu_iterator_is_done (itr);
       mu_iterator_next (itr))
    {
      mu_iterator_current (itr, (void **)&name);
      mu_printf ("%s%c", name, nl ? '\n' : ' ');
    }
  mu_iterator_destroy (&itr);
}

int
util_slist_lookup (mu_list_t list, const char *str)
{
  mu_iterator_t itr;
  char *name;
  int rc = 0;

  if (!list || mu_list_get_iterator (list, &itr))
    return 0;

  for (mu_iterator_first (itr); !mu_iterator_is_done (itr);
       mu_iterator_next (itr))
    {
      mu_iterator_current (itr, (void **)&name);
      if (mu_c_strcasecmp (name, str) == 0)
	{
	  rc = 1;
	  break;
	}
    }
  mu_iterator_destroy (&itr);
  return rc;
}

static int
util_slist_compare (const void *a, const void *b)
{
  return mu_c_strcasecmp (a, b);
}

void
util_slist_add (mu_list_t *plist, char *value)
{
  mu_list_t list;

  if (!*plist)
    {
      if (mu_list_create (&list))
	return;
      mu_list_set_destroy_item (list, mu_list_free_item);
      mu_list_set_comparator (list, util_slist_compare);
      *plist = list;
    }
  else
    list = *plist;
  mu_list_append (list, mu_strdup (value));
}

void
util_slist_remove (mu_list_t *list, char *value)
{
  mu_list_remove (*list, value);
}

void
util_slist_destroy (mu_list_t *list)
{
  mu_list_destroy (list);
}

char *
util_slist_to_string (mu_list_t list, const char *delim)
{
  mu_iterator_t itr;
  char *name;
  char *str = NULL;

  if (!list || mu_list_get_iterator (list, &itr))
    return NULL;

  for (mu_iterator_first (itr); !mu_iterator_is_done (itr);
       mu_iterator_next (itr))
    {
      mu_iterator_current (itr, (void **)&name);
      if (str && delim)
	util_strcat (&str, delim);
      util_strcat (&str, name);
    }
  mu_iterator_destroy (&itr);
  return str;
}

void
util_strcat (char **dest, const char *str)
{
  if (!*dest)
    *dest = mu_strdup (str);
  else
    {
      int dlen = strlen (*dest) + 1;
      int slen = strlen (str) + 1;
      char *newp = realloc (*dest, dlen + slen);

      if (!newp)
	return;

      *dest = newp;
      memcpy (newp + dlen - 1, str, slen);
    }
}

char *
util_outfolder_name (char *str)
{
  char *template = NULL;
  char *folder;
  char *outfolder;
  int rc;

  if (!str)
    {
      mailvar_get (&template, mailvar_name_record, mailvar_type_string, 0);
      str = template;
    }
  else
    {
      switch (*str)
	{
	case '/':
	case '~':
	case '+':
	  break;
	  
	default:
	  if (mailvar_is_true (mailvar_name_mailx))
	    {
	      if (mailvar_get (NULL, mailvar_name_outfolder,
			       mailvar_type_whatever, 0) == 0)
		{
		  if (mailvar_get (&folder, mailvar_name_folder,
				   mailvar_type_string, 0) == 0)
		    template = mu_make_file_name (folder, str);
		}
	      str = template;
	    }
	  else if (mailvar_get (&outfolder, mailvar_name_outfolder,
				mailvar_type_string, 0) == 0)
	    {
	      str = template = mu_make_file_name (outfolder, str);
	    }
	  else if (mailvar_is_true (mailvar_name_outfolder))
	    {
	      if (mailvar_get (&folder, mailvar_name_folder,
			       mailvar_type_string, 0) == 0)
		template = mu_make_file_name (folder, str);
	      str = template;
	    }
	  else
	    str = NULL;
	  break;
	}
    }
  
  if (str)
    {
      char *tilde_template = NULL;
      char *exp;

      if (mailvar_is_true (mailvar_name_mailx))
	{
	  switch (*str)
	    {
	    case '/':
	    case '~':
	    case '+':
	      break;
	      
	    default:
	      if (mu_asprintf (&tilde_template, "~/%s", str))
		{
		  mu_alloc_die ();
		}
	      str = tilde_template;
	    }
	}      
      
      rc = mu_mailbox_expand_name (str, &exp);
      if (rc)
	{
	  mu_diag_funcall (MU_DIAG_ERROR, "mu_mailbox_expand_name", str, rc);
	  str = NULL;
	}
      else
	str = exp;
      free (tilde_template);
    }
  free (template);
  
  return str;
}

/* Save an outgoing message. The SAVEFILE argument overrides the setting
   of the "record" variable. */
void
util_save_outgoing (mu_message_t msg, char *savefile)
{
  char *filename = util_outfolder_name (savefile);
  if (filename)
    {
      int rc;
      mu_mailbox_t outbox;

      rc = mu_mailbox_create_default (&outbox, filename);
      if (rc)
	{
	  mu_error (_("Cannot create output mailbox `%s': %s"),
		    filename, mu_strerror (rc));
	}
      else
	{
	  rc = mu_mailbox_open (outbox, MU_STREAM_WRITE | MU_STREAM_CREAT);
	  if (rc)
	    mu_error (_("Cannot open output mailbox `%s': %s"),
		      filename, mu_strerror (rc));
	  else
	    {
	      rc = mu_mailbox_append_message (outbox, msg);
	      if (rc)
		mu_error (_("Cannot append message to `%s': %s"),
			  filename, mu_strerror (rc));
	    }
	      
	  mu_mailbox_close (outbox);
	  mu_mailbox_destroy (&outbox);
	}
      free (filename);
    }
}

//FIXME
static int
util_descend_subparts (mu_message_t mesg, msgset_t *msgset, mu_message_t *part)
{
  unsigned int i;

  for (i = 2; i <= msgset_length (msgset); i++)
    {
      mu_message_t submsg = NULL;
      size_t nparts = 0;
      char *type = NULL;
      mu_header_t hdr = NULL;

      mu_message_get_header (mesg, &hdr);
      util_get_content_type (hdr, &type, NULL);
      if (mu_c_strncasecmp (type, "message/rfc822", strlen (type)) == 0)
	{
	  if (mu_message_unencapsulate (mesg, &submsg, NULL))
	    {
	      mu_error (_("Cannot unencapsulate message/part"));
	      return 1;
	    }
	  mesg = submsg;
	}

      mu_message_get_num_parts (mesg, &nparts);
      if (nparts < msgset->crd[i])
	{
	  mu_error (_("No such (sub)part in the message: %lu"),
		      (unsigned long) msgset->crd[i]);
	  return 1;
	}

      if (mu_message_get_part (mesg, msgset->crd[i], &submsg))
	{
	  mu_error (_("Cannot get (sub)part from the message: %lu"),
		      (unsigned long) msgset->crd[i]);
	  return 1;
	}

      mesg = submsg;
    }

  *part = mesg;
  return 0;
}

void
util_msgset_iterate (msgset_t *msgset,
		     int (*fun) (mu_message_t, msgset_t *, void *),
		     void *closure)
{
  for (; msgset; msgset = msgset->next)
    {
      mu_message_t mesg;

      if (mu_mailbox_get_message (mbox, msgset_msgno (msgset), &mesg) != 0)
	return;

      if (util_descend_subparts (mesg, msgset, &mesg) == 0)
	(*fun) (mesg, msgset, closure);
    }
}

void
util_get_content_type (mu_header_t hdr, char **value, char **args)
{
  char *type = NULL;
  util_get_hdr_value (hdr, MU_HEADER_CONTENT_TYPE, &type);
  if (type == NULL || *type == '\0')
    {
      if (type)
	free (type);
      type = mu_strdup ("text/plain"); /* Default.  */
    }
  else
    {
      char *p;
      p = strchr (type, ';');
      if (p)
	{
	  *p++ = 0;
	  if (args)
	    *args = p;
	}
    }
  *value = type;
}

void
util_get_hdr_value (mu_header_t hdr, const char *name, char **value)
{
  int status = mu_header_aget_value_unfold (hdr, name, value);
  if (status == 0)
    mu_rtrim_class (*value, MU_CTYPE_SPACE);
  else
    {
      if (status != MU_ERR_NOENT)
	mu_diag_funcall (MU_DIAG_ERROR, "mu_header_aget_value_unfold", name,
			 status);
      *value = NULL;
    }
}

int
util_merge_addresses (char **addr_str, const char *value)
{
  mu_address_t addr, new_addr;
  int rc;

  if ((rc = mu_address_create (&new_addr, value)) != 0)
    return rc;

  if ((rc = mu_address_create (&addr, *addr_str)) != 0)
    {
      mu_address_destroy (&new_addr);
      return rc;
    }

  rc = mu_address_union (&addr, new_addr);
  if (rc == 0)
    {
      char *val;

      rc = mu_address_aget_printable (addr, &val);
      if (rc == 0)
	{
	  if (!val)
	    return MU_ERR_NOENT;
	  free (*addr_str);
	  *addr_str = val;
	}
    }

  mu_address_destroy (&addr);
  mu_address_destroy (&new_addr);
  return rc;
}

int
is_address_field (const char *name)
{
  static char *address_fields[] = {
    MU_HEADER_TO,
    MU_HEADER_CC,
    MU_HEADER_BCC,
    0
  };
  char **p;

  for (p = address_fields; *p; p++)
    if (mu_c_strcasecmp (*p, name) == 0)
      return 1;
  return 0;
}

void
util_address_expand_aliases (mu_address_t *paddr)
{
  struct mu_address hint;
  mu_address_t addr = *paddr;
  mu_address_t new_addr = NULL;
  int rc;
  
  memset (&hint, 0, sizeof (hint));
  while (addr)
    {
      struct mu_address *next = addr->next;
      addr->next = NULL;

      if (addr->domain == NULL)
	{
	  char *exp = alias_expand (addr->local_part);
	  if (exp)
	    {
	      mu_address_destroy (&addr);
	      rc = mu_address_create_hint (&addr, exp, &hint, 0);
	      if (rc)
		{
		  mu_error (_("Cannot parse address `%s': %s"),
			    exp, mu_strerror (rc));
		  free (exp);
		  continue;
		}
	      free (exp);
	    }
	}
      mu_address_union (&new_addr, addr);
      mu_address_destroy (&addr);

      addr = next;
    }
  *paddr = new_addr;
}

int
util_header_expand_aliases (mu_header_t *phdr)
{
  size_t i, nfields = 0;
  mu_header_t hdr;
  int errcnt = 0, rc;

  if (mailvar_is_true (mailvar_name_inplacealiases))
    /* If inplacealiases was set, aliases have been already expanded */
    return 0;
  
  rc = mu_header_create (&hdr, "", 0);
  if (rc)
    {
      mu_error (_("Cannot create temporary header: %s"), mu_strerror (rc));
      return 1;
    }

  mu_header_get_field_count (*phdr, &nfields);
  for (i = 1; i <= nfields; i++)
    {
      const char *name;
      char *value;

      if (mu_header_sget_field_name (*phdr, i, &name))
	continue;

      if (mu_header_aget_field_value (*phdr, i, &value))
	continue;

      if (is_address_field (name))
	{
	  struct mu_address hint;
	  const char *s;
	  mu_address_t a = NULL;

	  mu_string_unfold (value, NULL);

	  memset (&hint, 0, sizeof (hint));
	  rc = mu_address_create_hint (&a, value, &hint, 0);
	  if (rc == 0)
	    {
	      mu_address_t ha = NULL;
	      util_address_expand_aliases (&a);
	      if (mu_header_sget_value (hdr, name, &s) == 0)
		mu_address_create_hint (&ha, s, &hint, 0);
	      mu_address_union (&ha, a);
	      mu_address_destroy (&a);
	      a = ha;
	    }
	  else
	    {
	      mu_error (_("Cannot parse address `%s': %s"),
			value, mu_strerror (rc));
	    }

	  if (a)
	    {
	      const char *newvalue;
		  
	      rc = mu_address_sget_printable (a, &newvalue);
	      if (rc == 0 && newvalue)
		mu_header_set_value (hdr, name, newvalue, 1);
	      mu_address_destroy (&a);
	    }
	}
      else
	mu_header_append (hdr, name, value);
      free (value);
    }

  if (errcnt == 0)
    {
      mu_header_destroy (phdr);
      *phdr = hdr;
    }
  else
    mu_header_destroy (&hdr);

  return errcnt;
}

int
util_get_message (mu_mailbox_t mbox, size_t msgno, mu_message_t *msg)
{
  int status;

  if (msgno > total)
    {
      util_error_range (msgno);
      return MU_ERR_NOENT;
    }

  status = mu_mailbox_get_message (mbox, msgno, msg);
  if (status)
    {
      mu_error (_("Cannot get message %lu: %s"),
		  (unsigned long) msgno, mu_strerror (status));
      return status;
    }

  return 0;
}

int
util_get_message_part (mu_mailbox_t mbox, msgset_t *msgset,
		       mu_message_t *ret_msg)
{
  int status, ismime;
  mu_message_t msg;
  size_t i;
  
  status = mu_mailbox_get_message (mbox, msgset_msgno (msgset), &msg);
  if (status)
    {
      mu_error (_("Cannot get message %lu: %s"),
		(unsigned long) msgset_msgno (msgset), mu_strerror (status));
      return status;
    }

  for (i = 2; i <= msgset_length (msgset); i++)
    {
      status = mu_message_is_multipart (msg, &ismime);
      if (status)
	{
	  mu_diag_funcall (MU_DIAG_ERROR, "mu_message_is_multipart",
			   NULL, status);
	  return status;
	}

      if (!ismime)
	{
	  char *s = msgset_part_str (msgset, i);
	  mu_error (_("%s: not a multipart message"), s);
	  free (s);
	  return status;
	}

      status = mu_message_get_part (msg, msgset->crd[i], &msg);
      if (status)
	{
	  mu_diag_funcall (MU_DIAG_ERROR, "mu_message_get_part",
			   NULL, status);
	  return status;
	}
    }
  *ret_msg = msg;
  return 0;
}

int
util_error_range (size_t msgno)
{
  mu_error (_("%lu: invalid message number"), (unsigned long) msgno);
  return 1;
}

void
util_noapp ()
{
  mu_error (_("No applicable messages"));
}

void
util_cache_command (mu_list_t *list, const char *fmt, ...)
{
  char *cmd = NULL;
  size_t size = 0;
  va_list ap;

  va_start (ap, fmt);
  mu_vasnprintf (&cmd, &size, fmt, ap);
  va_end (ap);

  if (!*list)
    mu_list_create (list);

  mu_list_append (*list, cmd);
}

static int
_run_and_free (void *item, void *data)
{
  util_do_command ("%s", (char *) item);
  free (item);
  return 0;
}

void
util_run_cached_commands (mu_list_t *list)
{
  mu_list_foreach (*list, _run_and_free, NULL);
  mu_list_destroy (list);
}

char *
util_get_charset (void)
{
  char *charset;

  if (mailvar_get (&charset, mailvar_name_charset, mailvar_type_string, 0))
    return NULL;

  if (mu_c_strcasecmp (charset, "auto") == 0)
    {
      struct mu_lc_all lc_all = { .flags = 0 };
      char *tmp = getenv ("LC_ALL");
      if (!tmp)
	tmp = getenv ("LANG");

      if (tmp && mu_parse_lc_all (tmp, &lc_all, MU_LC_CSET) == 0)
	{
	  charset = mu_strdup (lc_all.charset);
	  mu_lc_all_free (&lc_all);
	}
      else
	charset = NULL;
    }
  else
    charset = mu_strdup (charset);

  return charset;
}

void
util_rfc2047_decode (char **value)
{
  char *charset, *tmp;
  int rc;

  if (!*value)
    return;
  charset = util_get_charset ();
  if (!charset)
    return;

  rc = mu_rfc2047_decode (charset, *value, &tmp);
  free (charset);

  if (rc)
    {
      if (mailvar_is_true (mailvar_name_verbose))
	mu_error (_("Cannot decode line `%s': %s"), *value, mu_strerror (rc));
    }
  else
    {
      free (*value);
      *value = tmp;
    }
}

const char *
util_url_to_string (mu_url_t url)
{
  const char *scheme;
  if (mu_url_sget_scheme (url, &scheme) == 0)
    {
      if (strcmp (scheme, "file") == 0 || strcmp (scheme, "mbox") == 0)
	{
	  const char *path;
	  if (mu_url_sget_path (url, &path) == 0)
	    return path;
	}
    }
  return mu_url_to_string (url);
}

mu_stream_t
open_pager (size_t lines)
{
  const char *pager;
  unsigned pagelines = util_get_crt ();
  mu_stream_t str;

  if (pagelines && lines > pagelines && (pager = getenv ("PAGER")))
    {
      int rc = mu_command_stream_create (&str, pager, MU_STREAM_WRITE);
      if (rc)
	{
	  mu_diag_funcall (MU_DIAG_ERROR, "mu_prog_stream_create",
			   pager, rc);
	  str = mu_strout;
	  mu_stream_ref (str);
	}
    }
  else
    {
      str = mu_strout;
      mu_stream_ref (str);
    }
  return str;
}

int
util_get_folder (mu_folder_t *pfolder, mu_url_t url, int type)
{
  mu_folder_t folder;
  int rc;

  rc = mu_folder_create_from_record (&folder, url, NULL);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_folder_create",
		       mu_url_to_string (url), rc);
      return -1;
    }

  if (!mu_folder_is_local (folder))
    {
      if (type == local_folder)
	{
	  /* TRANSLATORS: The subject of this sentence ("folder") is the
	     name of the variable. Don't translate it. */
	  mu_error ("%s", _("folder must be set to a local folder"));
	  mu_folder_destroy (&folder);
	  return -1;
	}

      /* Set ticket for a remote folder */
      rc = mu_folder_attach_ticket (folder);
      if (rc)
	{
	  mu_authority_t auth = NULL;

	  if (mu_folder_get_authority (folder, &auth) == 0 && auth)
	    {
	      mu_ticket_t tct;
	      mu_noauth_ticket_create (&tct);
	      rc = mu_authority_set_ticket (auth, tct);
	      if (rc)
		mu_diag_funcall (MU_DIAG_ERROR, "mu_authority_set_ticket",
				 NULL, rc);
	    }
	}
    }

  rc = mu_folder_open (folder, MU_STREAM_READ);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_folder_open",
		       mu_url_to_string (url), rc);
      mu_folder_destroy (&folder);
      return -1;
    }

  *pfolder = folder;
  return 0;
}

