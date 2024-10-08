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

#include "imap4d.h"
#include <dirent.h>
#include <pwd.h>

/*
 * IMPORTANT NOTE:
 *
 * The LIST command takes two arguments: 'reference' and 'mailbox',
 * which in the code below is referred to as 'wcard'.  Now, RFC 3501
 * states, that:
 *
 *  If the reference argument is not a level of mailbox
 *  hierarchy (that is, it is a \NoInferiors name), and/or
 *  the reference argument does not end with the hierarchy
 *  delimiter, it is implementation-dependent how this is
 *  interpreted.  For example, a reference of "foo/bar" and
 *  mailbox name of "rag/baz" could be interpreted as
 *  "foo/bar/rag/baz", "foo/barrag/baz", or "foo/rag/baz".
 *  A client SHOULD NOT use such a reference argument except
 *  at the explicit request of the user.  A hierarchical
 *  browser MUST NOT make any assumptions about server
 *  interpretation of the reference unless the reference is
 *  a level of mailbox hierarchy AND ends with the hierarchy
 *  delimiter.
 *
 * Mailutils' approach is basically to concatenate the two
 * arguments with a hierarchy separator (as per RFC 2342) in
 * between.  In detail:
 *
 *  1. Given two arguments, 'reference' and 'wcard', the reference is
 *     used to look up the matching namespace (first approximation).
 *  2. If wcard contains non-wildcard directory prefix, that prefix
 *     is removed and appended to the reference, separated by the
 *     namespace delimiter.
 *  3. The updated reference is used to look up the final namespace.
 *  4. If the namespace prefix ends with a delimiter, wcard is appended
 *     to it.
 *  5. Otherwise, the name to look up is formed by concatenating the
 *     namespace prefix, namespace delimiter, and wcard.
 */

struct refinfo
{
  char const *refptr;   /* Original reference */
  size_t reflen;        /* Length of the original reference */
  struct namespace_prefix const *pfx;
  int delim;            /* If not 0, this character will be inserted between
			   the original reference and mailbox name */
  char *buf;
  size_t bufsize;
};

static int
list_fun (mu_folder_t folder, struct mu_list_response *resp, void *data)
{
  char *name;
  struct refinfo *refinfo = data;
  size_t size;
  char *p;

  name = resp->name;

  /* There can be only one INBOX */
  if (refinfo->reflen == 0 &&  mu_c_strcasecmp (name, "INBOX") == 0)
    return 0;

  /* Ignore mailboxes that contain delimiter as part of their name */
  if (refinfo->pfx->delim != resp->separator
      && strchr (name, refinfo->pfx->delim))
    return 0;
  
  io_sendf ("* %s", "LIST (");
  if ((resp->type & (MU_FOLDER_ATTRIBUTE_FILE|MU_FOLDER_ATTRIBUTE_DIRECTORY))
       == (MU_FOLDER_ATTRIBUTE_FILE|MU_FOLDER_ATTRIBUTE_DIRECTORY))
    /* nothing */;
  else if (resp->type & MU_FOLDER_ATTRIBUTE_FILE)
    io_sendf ("\\NoInferiors");
  else if (resp->type & MU_FOLDER_ATTRIBUTE_DIRECTORY)
    io_sendf ("\\NoSelect");
  
  io_sendf (") \"%c\" ", refinfo->pfx->delim);

  size = strlen (name) + refinfo->reflen + 2;
  if (size > refinfo->bufsize)
    {
      if (refinfo->buf == NULL)
	{
	  refinfo->bufsize = size;
	  refinfo->buf = mu_alloc (refinfo->bufsize);
	}
      else
	{
	  refinfo->buf = mu_realloc (refinfo->buf, size);
	  refinfo->bufsize = size;
	}
    }

  if (refinfo->refptr[0])
    {
      memcpy (refinfo->buf, refinfo->refptr, refinfo->reflen);
      p = refinfo->buf + refinfo->reflen;
      if (refinfo->delim)
	*p++ = refinfo->delim;
    }
  else
    p = refinfo->buf;
  if (*name)
    translate_delim (p, name, refinfo->pfx->delim, resp->separator);
  
  name = refinfo->buf;
  
  if (strpbrk (name, "\"{}"))
    io_sendf ("{%lu}\n%s\n", (unsigned long) strlen (name), name);
  else if (is_atom (name))
    io_sendf ("%s\n", name);
  else
    {
      io_send_astring (name);
      io_sendf ("\n");
    }
  return 0;
}

/* Return 1 if the string REF matches exactly the prefix in PFX. */
static int
match_pfx (struct namespace_prefix const *pfx, char const *ref)
{
  char const *p = pfx->prefix, *q = ref;

  for (; *q; p++, q++)
    {
      if (*p == 0)
	return *q == pfx->delim && q[1] == 0;
      if (*p != *q)
	return 0;
    }
  if (*p == pfx->delim)
    p++;
  return *p == 0;
}

static int
list_ref (char const *ref, char const *wcard, char const *cwd,
	  struct namespace_prefix const *pfx)
{
  int rc;
  struct refinfo refinfo;
  mu_folder_t folder;
  struct mu_folder_scanner scn = MU_FOLDER_SCANNER_INITIALIZER;

  if (!wcard[0])
    {
      /* An empty ("" string) mailbox name argument is a special request to
	 return the hierarchy delimiter and the root name of the name given
	 in the reference.
      */ 
      io_sendf ("* LIST (\\NoSelect) ");
      if (mu_c_strcasecmp (ref, "INBOX") == 0)
	{
	  io_sendf ("NIL \"\"");
	}
      else
	{
	  io_sendf ("\"%c\" ", pfx->delim);
	  io_send_astring (pfx->prefix);
	}
      io_sendf ("\n");
      return RESP_OK;
    }
  
  if (pfx->ns == NS_OTHER && match_pfx (pfx, ref) && strpbrk (wcard, "*%"))
    {
      /* [A] server MAY return NO to such a LIST command, requiring that a
	 user name be included with the Other Users' Namespace prefix
	 before listing any other user's mailboxes */
      return RESP_NO;
    }	  
	
  rc = mu_folder_create (&folder, cwd);
  if (rc)
    return RESP_NO;

  /* Force the right matcher */
  mu_folder_set_match (folder, mu_folder_imap_match);

  memset (&refinfo, 0, sizeof refinfo);

  refinfo.pfx = pfx;
  /* Note: original reference would always coincide with the pfx->prefix,
     if it weren't for the special handling of NS_OTHER namespace, where
     the part between the prefix and the first delimiter is considered to
     be a user name and is handled as part of the actual prefix. */
  refinfo.refptr = ref;
  refinfo.reflen = strlen (ref);

  /* Insert delimiter after the reference prefix, unless the latter already
     ends with a delimiter or is the same as the namespace prefix. */
  if (ref[refinfo.reflen-1] != pfx->delim && strcmp (ref, pfx->prefix))
    refinfo.delim = pfx->delim;
  
  /* The special name INBOX is included in the output from LIST, if
     INBOX is supported by this server for this user and if the
     uppercase string "INBOX" matches the interpreted reference and
     mailbox name arguments with wildcards as described above.  The
     criteria for omitting INBOX is whether SELECT INBOX will return
     failure; it is not relevant whether the user's real INBOX resides
     on this or some other server. */
  
  if (!*ref &&
      mu_imap_wildmatch_ci (wcard, "INBOX", MU_HIERARCHY_DELIMITER) == 0)
    io_untagged_response (RESP_NONE, "LIST (\\NoInferiors) NIL INBOX");

  scn.pattern = namespace_decode_delim (pfx, wcard);
  scn.enumfun = list_fun;
  scn.enumdata = &refinfo;
  if (refinfo.pfx->record)
    {
      mu_list_create (&scn.records);
      mu_list_append (scn.records, refinfo.pfx->record);
    }
  
  mu_folder_scan (folder, &scn);
  free (scn.pattern);
  mu_list_destroy (&scn.records);
  mu_folder_destroy (&folder);
  free (refinfo.buf);
  return RESP_OK;
}

/*
6.3.8.  LIST Command

   Arguments:  reference name
               mailbox name with possible wildcards

   Responses:  untagged responses: LIST

   Result:     OK - list completed
               NO - list failure: can't list that reference or name
               BAD - command unknown or arguments invalid
*/

/*
  1- IMAP4 insists: the reference argument present in the
  interpreted form SHOULD prefix the interpreted form.  It SHOULD
  also be in the same form as the reference name argument.  This
  rule permits the client to determine if the returned mailbox name
  is in the context of the reference argument, or if something about
  the mailbox argument overrode the reference argument.
  
  ex:
  Reference         Mailbox         -->  Interpretation
  ~smith/Mail        foo.*          -->  ~smith/Mail/foo.*
  archive            %              --> archive/%
  #news              comp.mail.*     --> #news.comp.mail.*
  ~smith/Mail        /usr/doc/foo   --> /usr/doc/foo
  archive            ~fred/Mail     --> ~fred/Mail/ *

  2- The character "*" is a wildcard, and matches zero or more characters
  at this position.  The character "%" is similar to "*",
  but it does not match the hierarchy delimiter.  */

int
imap4d_list (struct imap4d_session *session,
             struct imap4d_command *command, imap4d_tokbuf_t tok)
{
  char *ref;
  char *wcard;
  int status = RESP_OK;
  static char *resp_text[] = {
    [RESP_OK]  = "Completed",
    [RESP_NO]  = "The requested item could not be found",
    [RESP_BAD] = "System error"
  };
  char *cwd = NULL;
  struct namespace_prefix const *pfx = NULL;
      
  if (imap4d_tokbuf_argc (tok) != 4)
    return io_completion_response (command, RESP_BAD, "Invalid arguments");
  
  ref = imap4d_tokbuf_getarg (tok, IMAP4_ARG_1);
  wcard = imap4d_tokbuf_getarg (tok, IMAP4_ARG_2);

  if (ref[0] == 0)
    {
      cwd = namespace_translate_name (wcard, &pfx);
      if (cwd)
	{
	  size_t p_len = strlen (pfx->prefix);
	  size_t w_len = strlen (wcard);
	  
	  if (p_len <= w_len)
	    {
	      memmove (wcard, wcard + p_len, w_len - p_len + 1);
	      ref = mu_strdup (pfx->prefix);
	    }
	  else
	    ref = mu_strdup (ref);
	  free (cwd);
	}
      else
	ref = mu_strdup (ref);
    }
  else
    ref = mu_strdup (ref);
  
  if (!pfx)
    {
      cwd = namespace_translate_name (ref, &pfx);
      if (cwd)
	free (cwd);
    }
  
  if (pfx)
    {
      /* Find the longest directory prefix */
      size_t i = strcspn (wcard, "%*");
      if (wcard[i])
	{
	  while (i > 0 && wcard[i - 1] != pfx->delim)
	    i--;
	  /* Append it to the reference */
	  if (i)
	    {
	      size_t reflen = strlen (ref);
	      size_t len = i + reflen;
	      
	      ref = mu_realloc (ref, len);
	      memcpy (ref + reflen, wcard, i - 1); /* omit the trailing / */
	      ref[len-1] = 0;
	      
	      wcard += i;
	    }
	}
    }
	  
  cwd = namespace_translate_name (ref, &pfx);
  if (cwd)
    status = list_ref (ref, wcard, cwd, pfx);
  else
    status = RESP_NO;

  free (cwd);
  free (ref);
  
  return io_completion_response (command, status, "%s", resp_text[status]);
}

