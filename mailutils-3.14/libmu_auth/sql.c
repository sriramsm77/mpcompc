/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2002-2022 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General
   Public License along with this library.  If not, see 
   <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#ifdef HAVE_SHADOW_H
# include <shadow.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#else
# include <string.h>
#endif
#ifdef HAVE_CRYPT_H
# include <crypt.h>
#endif

#include <mailutils/assoc.h>
#include <mailutils/list.h>
#include <mailutils/iterator.h>
#include <mailutils/mailbox.h>
#include <mailutils/mu_auth.h>
#include <mailutils/error.h>
#include <mailutils/errno.h>
#include <mailutils/nls.h>
#include <mailutils/util.h>
#include <mailutils/sql.h>
#include <mailutils/cstr.h>
#include <mailutils/wordsplit.h>
#include <mailutils/cli.h>
#include <mailutils/kwd.h>
#include "sql.h"

#ifdef USE_SQL

struct mu_sql_module_config mu_sql_module_config;

/* Resource file configuration */

static struct mu_kwd password_encryption[] = {
  { "plain", mu_sql_password_plaintext },
  { "scrambled", mu_sql_password_scrambled },
  { "hash", mu_sql_password_hash },
  { "crypt", mu_sql_password_hash },
  { NULL }
};

static int
cb_password_encryption (void *data, mu_config_value_t *val)
{
  int res;
  
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;

  if (mu_kwd_xlat_name (password_encryption, val->v.string, &res))
    mu_error ("%s", _("unrecognized password encryption"));
  else
    mu_sql_module_config.password_encryption = res;
  return 0;
}

static int
cb_field_map (void *data, mu_config_value_t *val)
{
  char *err_term;
  int rc = mu_cfg_field_map (val, &mu_sql_module_config.field_map, &err_term);

  if (rc)
    {
      if (err_term)
	mu_error (_("error near %s: %s"), err_term, mu_strerror (rc));
      else
	mu_error ("%s", mu_strerror (rc));
    }

  return rc;
}

static int
cb_interface (void *data, mu_config_value_t *val)
{
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  mu_sql_module_config.interface = mu_sql_interface_index (val->v.string);
  if (mu_sql_module_config.interface == 0)
    {
      mu_error (_("unknown SQL interface `%s'"), val->v.string);
      return 1;
    }
  return 0;
}
  
static struct mu_cfg_param mu_sql_param[] = {
  { "interface", mu_cfg_callback, &mu_sql_module_config.interface, 0,
    cb_interface,
    N_("Set SQL interface to use."),
    /* TRANSLATORS: Words to the right of : are keywords - do not translate */
    N_("iface: mysql|odbc|postgres") },
  { "getpwnam", mu_c_string, &mu_sql_module_config.getpwnam_query, 0, NULL,
    N_("SQL query to use for getpwnam requests."),
    N_("query") },
  { "getpwuid", mu_c_string, &mu_sql_module_config.getpwuid_query, 0, NULL,
    N_("SQL query to use for getpwuid requests."),
    N_("query") },
  { "getpass", mu_c_string, &mu_sql_module_config.getpass_query, 0, NULL,
    N_("SQL query returning the user's password."),
    N_("query") },
  { "host", mu_c_string, &mu_sql_module_config.host, 0, NULL,
    N_("SQL server host name.") },
  { "user", mu_c_string, &mu_sql_module_config.user, 0, NULL,
    N_("SQL user name.") },
  { "passwd", mu_c_string, &mu_sql_module_config.passwd, 0, NULL,
    N_("Password for the SQL user.") },
  { "port", mu_c_int, &mu_sql_module_config.port, 0, NULL,
    N_("SQL server port.") },
  { "db", mu_c_string, &mu_sql_module_config.db, 0, NULL,
    N_("Database name.") },
  { "password-encryption", mu_cfg_callback, NULL, 0, cb_password_encryption,
    N_("Type of password returned by getpass query."),
    /* TRANSLATORS: Words to the right of : are keywords - do not translate */
    N_("arg: plain|hash|crypt|scrambled") },
  { "field-map", mu_cfg_callback, NULL, 0, cb_field_map,
    N_("Set a field-map for parsing SQL replies.  The map is a "
       "column-separated list of definitions.  Each definition has the "
       "following form:\n"
       "   <name: string>=<column: string>\n"
       "where <name> is one of the following: name, passwd, uid, gid, "
       "gecos, dir, shell, mailbox, quota, and <column> is the name of "
       "the corresponding SQL column."),
    N_("map: definition") },
  { "param", mu_c_string, &mu_sql_module_config.param, 0, NULL,
    N_("Extra parameters for connection (backend-specific)"),
    N_("arg") },
  { NULL }
};


static char *
sql_escape_string (const char *ustr)
{
  char *str, *q;
  const unsigned char *p;
  size_t len = strlen (ustr);
#define ESCAPABLE_CHAR "\\'\""
  
  for (p = (const unsigned char *) ustr; *p; p++)
    {
      if (strchr (ESCAPABLE_CHAR, *p))
	len++;
    }

  str = malloc (len + 1);
  if (!str)
    return NULL;

  for (p = (const unsigned char *) ustr, q = str; *p; p++)
    {
      if (strchr (ESCAPABLE_CHAR, *p))
	*q++ = '\\';
      *q++ = *p;
    }
  *q = 0;
  return str;
}

char *
mu_sql_expand_query (const char *query, const char *ustr)
{
  int rc;
  char *res;
  char *esc_ustr;

  if (!query)
    return NULL;

  esc_ustr = sql_escape_string (ustr);
  rc = mu_str_vexpand (&res, query, "user", esc_ustr, NULL);
  free (esc_ustr);
  if (rc)
    {
      if (rc == MU_ERR_FAILURE)
	{
	  mu_error (_("cannot expand line `%s': %s"), query, res);
	  free (res);
	}
      else
	mu_error (_("cannot expand line `%s': %s"), query, mu_strerror (rc));
      return NULL;
    }
  return res;
}

static int
get_field (mu_sql_connection_t conn, const char *id, char **ret, int mandatory)
{
  int rc;
  const char *name = mu_assoc_get (mu_sql_module_config.field_map, id);
  if (!name)
    name = id;
  rc = mu_sql_get_field (conn, 0, name, ret);
  if (rc)
    {
      if (mandatory || rc != MU_ERR_NOENT)
	mu_error (_("cannot get SQL field `%s' (`%s'): %s"),
		  id, name, mu_strerror (rc));
    }
  else if (!*ret)
    {
      if (mandatory)
	{
	  mu_error (_("SQL field `%s' (`%s') has NULL value"),
		    id, name);
	  rc = MU_ERR_READ;
	}
      else
	rc = MU_ERR_NOENT;
    }

  return rc;
}

static int
decode_tuple (mu_sql_connection_t conn, int n,
	      struct mu_auth_data **return_data)
{
  int rc;
  char *mailbox_name = NULL;
  char *name;
  char *passwd, *suid, *sgid, *dir, *shell, *gecos, *squota;
  mu_off_t quota = 0;
  char *p;
  uid_t uid;
  gid_t gid;

  if (get_field (conn, MU_AUTH_NAME, &name, 1)
      || get_field (conn, MU_AUTH_PASSWD, &passwd, 1)
      || get_field (conn, MU_AUTH_UID, &suid, 1)
      || get_field (conn, MU_AUTH_GID, &sgid, 1)     
      || get_field (conn, MU_AUTH_DIR, &dir, 1)     
      || get_field (conn, MU_AUTH_SHELL, &shell, 1))
    return MU_ERR_FAILURE;

  if (get_field (conn, MU_AUTH_GECOS, &gecos, 0))
    gecos = "SQL user";
  
  uid = strtoul (suid, &p, 0);
  if (*p)
    {
      mu_error (_("invalid value for uid: %s"), suid);
      return MU_ERR_FAILURE;
    }

  gid = strtoul (sgid, &p, 0);
  if (*p)
    {
      mu_error (_("invalid value for gid: %s"), sgid);
      return MU_ERR_FAILURE;
    }
  
  rc = get_field (conn, MU_AUTH_MAILBOX, &mailbox_name, 0);
  switch (rc)
    {
    case 0:
      mailbox_name = strdup (mailbox_name);
      if (!mailbox_name)
        return ENOMEM;
      break;
      
    case MU_ERR_NOENT:
      if (mu_construct_user_mailbox_url (&mailbox_name, name))
	return MU_ERR_FAILURE;
      break;

    default:
      return MU_ERR_FAILURE;
    }

  rc = get_field (conn, MU_AUTH_QUOTA, &squota, 0);
  if (rc == 0)
    {
      if (mu_c_strcasecmp (squota, "none") == 0)
	quota = 0;
      else
	{
	  quota = strtoul (squota, &p, 10);
	  switch (*p)
	    {
	    case 0:
	      break;
	      
	    case 'k':
	    case 'K':
	      quota *= 1024;
	      break;
      
	    case 'm':
	    case 'M':
	      quota *= 1024*1024;
	      break;
	      
	    default:
	      mu_error (_("invalid value for quota: %s"), squota);
	      free (mailbox_name);
	      return MU_ERR_FAILURE;
	    }
	}
    }
  else if (rc ==  MU_ERR_NOENT)
    quota = 0;
  else
    {
      free (mailbox_name);
      return MU_ERR_FAILURE;
    }

  rc = mu_auth_data_alloc (return_data,
			   name,
			   passwd,
			   uid,
			   gid,
			   gecos,
			   dir,
			   shell,
			   mailbox_name,
			   1);
  if (rc == 0)
    mu_auth_data_set_quota (*return_data, quota);
  
  free (mailbox_name);
  return rc;
}  

static int
mu_auth_sql_by_name (struct mu_auth_data **return_data,
		     const void *key,
		     void *func_data MU_ARG_UNUSED,
		     void *call_data MU_ARG_UNUSED)
{
  int status, rc;
  char *query_str = NULL;
  mu_sql_connection_t conn;
  size_t n;
  
  if (!key)
    return EINVAL;

  query_str = mu_sql_expand_query (mu_sql_module_config.getpwnam_query, key);

  if (!query_str)
    return MU_ERR_FAILURE;

  status = mu_sql_connection_init (&conn,
				   mu_sql_module_config.interface,
				   mu_sql_module_config.host,
				   mu_sql_module_config.port,
				   mu_sql_module_config.user,
				   mu_sql_module_config.passwd,
				   mu_sql_module_config.db,
				   mu_sql_module_config.param);

  if (status)
    {
      mu_error ("%s: %s", mu_strerror (status), mu_sql_strerror (conn));
      mu_sql_connection_destroy (&conn);
      free (query_str);
      return MU_ERR_FAILURE;
    }

  status = mu_sql_connect (conn);

  if (status)
    {
      mu_error ("%s: %s", mu_strerror (status), mu_sql_strerror (conn));
      mu_sql_connection_destroy (&conn);
      free (query_str);
      return EAGAIN;
    }
  
  status = mu_sql_query (conn, query_str);
  free (query_str);
  
  if (status)
    {
      mu_error (_("SQL query failed: %s"),
		(status == MU_ERR_SQL) ?  mu_sql_strerror (conn) :
	 	                          mu_strerror (status));
      mu_sql_connection_destroy (&conn);
      return MU_ERR_FAILURE;
    }

  status = mu_sql_store_result (conn);

  if (status)
    {
      mu_error (_("cannot store SQL result: %s"),
		(status == MU_ERR_SQL) ?  mu_sql_strerror (conn) :
	 	                          mu_strerror (status));
      mu_sql_connection_destroy (&conn);
      return MU_ERR_FAILURE;
    }

  status = mu_sql_num_tuples (conn, &n);
  if (status)
    {
      mu_error (_("cannot get number of tuples: %s"),
                (status == MU_ERR_SQL) ?  mu_sql_strerror (conn) :
                                          mu_strerror (status));
      mu_sql_release_result (conn);
      mu_sql_connection_destroy (&conn);
      return MU_ERR_FAILURE;
    }
  
  if (n == 0)
    rc = MU_ERR_AUTH_FAILURE;
  else
    rc = decode_tuple (conn, n, return_data);
  
  mu_sql_release_result (conn);
  mu_sql_disconnect (conn);
  mu_sql_connection_destroy (&conn);
  
  return rc;
}

static int
mu_auth_sql_by_uid (struct mu_auth_data **return_data,
		    const void *key,
		    void *func_data MU_ARG_UNUSED,
		    void *call_data MU_ARG_UNUSED)
{
  char uidstr[64];
  int status, rc;
  char *query_str = NULL;
  mu_sql_connection_t conn;
  size_t n;
  
  if (!key)
    return EINVAL;

  snprintf (uidstr, sizeof (uidstr), "%u", *(uid_t*)key);
  query_str = mu_sql_expand_query (mu_sql_module_config.getpwuid_query,
				   uidstr);

  if (!query_str)
    return ENOMEM;

  status = mu_sql_connection_init (&conn,
				   mu_sql_module_config.interface,
				   mu_sql_module_config.host,
				   mu_sql_module_config.port,
				   mu_sql_module_config.user,
				   mu_sql_module_config.passwd,
				   mu_sql_module_config.db,
				   mu_sql_module_config.param);

  if (status)
    {
      mu_error ("%s: %s", mu_strerror (status), mu_sql_strerror (conn));
      mu_sql_connection_destroy (&conn);
      free (query_str);
      return MU_ERR_FAILURE;
    }

  status = mu_sql_connect (conn);

  if (status)
    {
      mu_error ("%s: %s", mu_strerror (status), mu_sql_strerror (conn));
      mu_sql_connection_destroy (&conn);
      free (query_str);
      return EAGAIN;
    }
  
  status = mu_sql_query (conn, query_str);
  free (query_str);
  
  if (status)
    {
      mu_error (_("SQL query failed: %s"),
		(status == MU_ERR_SQL) ?  mu_sql_strerror (conn) :
	 	                          mu_strerror (status));
      mu_sql_connection_destroy (&conn);
      return MU_ERR_FAILURE;
    }

  status = mu_sql_store_result (conn);

  if (status)
    {
      mu_error (_("cannot store SQL result: %s"),
		(status == MU_ERR_SQL) ?  mu_sql_strerror (conn) :
	 	                          mu_strerror (status));
      mu_sql_connection_destroy (&conn);
      return MU_ERR_FAILURE;
    }

  status = mu_sql_num_tuples (conn, &n);
  if (status)
    {
      mu_error (_("cannot get number of tuples: %s"),
                (status == MU_ERR_SQL) ?  mu_sql_strerror (conn) :
                                          mu_strerror (status));
      mu_sql_release_result (conn);
      mu_sql_connection_destroy (&conn);
      return MU_ERR_FAILURE;
    }

  if (n == 0)
    rc = MU_ERR_AUTH_FAILURE;
  else
    rc = decode_tuple (conn, n, return_data);
  
  mu_sql_release_result (conn);
  mu_sql_disconnect (conn);
  mu_sql_connection_destroy (&conn);
  
  return rc;
}

int
mu_sql_getpass (const char *username, char **passwd)
{
  mu_sql_connection_t conn;
  char *query_str;
  int status;
  char *sql_pass;
  size_t nt;

  query_str = mu_sql_expand_query (mu_sql_module_config.getpass_query, username);

  if (!query_str)
    return MU_ERR_FAILURE;

  status = mu_sql_connection_init (&conn,
				   mu_sql_module_config.interface,
				   mu_sql_module_config.host,
				   mu_sql_module_config.port,
				   mu_sql_module_config.user,
				   mu_sql_module_config.passwd,
				   mu_sql_module_config.db,
				   mu_sql_module_config.param);

  if (status)
    {
      mu_error ("%s: %s", mu_strerror (status), mu_sql_strerror (conn));
      mu_sql_connection_destroy (&conn);
      free (query_str);
      return MU_ERR_FAILURE;
    }

  status = mu_sql_connect (conn);

  if (status)
    {
      mu_error ("%s: %s", mu_strerror (status), mu_sql_strerror (conn));
      mu_sql_connection_destroy (&conn);
      free (query_str);
      return EAGAIN;
    }
  
  status = mu_sql_query (conn, query_str);
  free (query_str);
  
  if (status)
    {
      mu_error (_("SQL query failed: %s"),
		(status == MU_ERR_SQL) ?  mu_sql_strerror (conn) :
	 	                          mu_strerror (status));
      mu_sql_connection_destroy (&conn);
      return MU_ERR_FAILURE;
    }
  
  status = mu_sql_store_result (conn);

  if (status)
    {
      mu_error (_("cannot store SQL result: %s"),
		(status == MU_ERR_SQL) ?  mu_sql_strerror (conn) :
	 	                          mu_strerror (status));
      mu_sql_connection_destroy (&conn);
      return MU_ERR_FAILURE;
    }

  status = mu_sql_num_tuples (conn, &nt);
  if (status)
    {
      mu_error (_("cannot get number of tuples: %s"),
                (status == MU_ERR_SQL) ?  mu_sql_strerror (conn) :
                                          mu_strerror (status));
      mu_sql_release_result (conn);
      mu_sql_connection_destroy (&conn);
      return MU_ERR_FAILURE;
    }
  if (nt == 0)
    {
      mu_sql_release_result (conn);
      mu_sql_connection_destroy (&conn);
      return MU_ERR_FAILURE;
    }

  status = mu_sql_get_column (conn, 0, 0, &sql_pass);
  if (status)
    {
      mu_error (_("cannot get password from SQL: %s"),
		(status == MU_ERR_SQL) ?  mu_sql_strerror (conn) :
	 	                          mu_strerror (status));
      mu_sql_release_result (conn);
      mu_sql_connection_destroy (&conn);
      return MU_ERR_FAILURE;
    }

  if (!sql_pass)
    {
      mu_error (_("SQL returned NULL password"));
      mu_sql_release_result (conn);
      mu_sql_connection_destroy (&conn);
      return MU_ERR_FAILURE;
    }

  *passwd = strdup (sql_pass);

  mu_sql_disconnect (conn);
  mu_sql_connection_destroy (&conn);

  if (!*passwd)
    return ENOMEM;

  return 0;
}

static int
mu_sql_authenticate (struct mu_auth_data **return_data MU_ARG_UNUSED,
		     const void *key,
		     void *func_data MU_ARG_UNUSED, void *call_data)
{
  const struct mu_auth_data *auth_data = key;
  char *pass = call_data;
  char *sql_pass, *crypt_pass;
  int rc;
  
  if (!auth_data)
    return EINVAL;

  if ((rc = mu_sql_getpass (auth_data->name, &sql_pass)))
    return rc;

  switch (mu_sql_module_config.password_encryption)
    {
    case mu_sql_password_hash:
      crypt_pass = crypt (pass, sql_pass);
      if (!crypt_pass)
        rc = 1;
      else
        rc = strcmp (sql_pass, crypt_pass);
      break;

    case mu_sql_password_scrambled:
      /* FIXME: Should this call be implementation-independent? I mean,
         should we have mu_sql_check_scrambled() that will match the
	 password depending on the exact type of the underlying database,
	 just as the rest of mu_sql_.* functions do */
#ifdef HAVE_MYSQL
      rc = mu_check_mysql_scrambled_password (sql_pass, pass);
#else
      rc = 1;
#endif
      break;

    case mu_sql_password_plaintext:
      rc = strcmp (sql_pass, pass);
      break;
    }

  free (sql_pass);
  
  return rc == 0 ? 0 : MU_ERR_AUTH_FAILURE;
}

#else

# define mu_sql_authenticate mu_auth_nosupport
# define mu_auth_sql_by_name mu_auth_nosupport
# define mu_auth_sql_by_uid mu_auth_nosupport
# define mu_sql_param NULL
#endif


struct mu_auth_module mu_auth_sql_module = {
  .name = "sql",
  .cfg = mu_sql_param,
  .handler = {
    [mu_auth_authenticate] = mu_sql_authenticate,
    [mu_auth_getpwnam]     = mu_auth_sql_by_name,
    [mu_auth_getpwuid]     = mu_auth_sql_by_uid
  }
};

