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

#include "libmda.h"
#include <mailutils/dbm.h>

static char *quotadbname;
#ifdef USE_SQL
static char *quota_query;
#endif

static int ex_quota_tempfail;

int
ex_quota ()
{
  return (ex_quota_tempfail ? EX_TEMPFAIL : EX_UNAVAILABLE);
}


struct mu_cfg_param mda_mailquota_cfg[] = {
  { "database", mu_c_string, &quotadbname, 0, NULL,
    N_("Name of DBM quota database file."),
    N_("file") },
#ifdef USE_SQL
  { "sql-query", mu_c_string, &quota_query, 0, NULL,
    N_("SQL query to retrieve mailbox quota."),
    N_("query") },
#endif
  { "exit-tempfail", mu_c_bool, &ex_quota_tempfail, 0, NULL,
    N_("Indicate temporary failure if the recipient is over his mail quota.")
  },
  { NULL }
};

#define DEFRETVAL MQUOTA_UNLIMITED

mu_off_t groupquota = 5*1024*1024UL;

static int
get_quota (mu_off_t *pquota, const char *str)
{
  size_t n;
  char *p;
  int rc = mu_strtosize (str, &p, &n);
  switch (rc)
    {
    case 0:
      *pquota = n;
      break;
      
    case ERANGE:
      mu_error (_("quota value is out of allowed range: %s"), str);
      break;

    case MU_ERR_PARSE:
      mu_error (_("bad quota value: %s, stopped at %s"), str, p);
      break;

    default:
      mu_diag_funcall (MU_DIAG_ERROR, "mu_strtosize", str, rc);
    }
  return rc;
}

enum {
	RETR_OK,
	RETR_UNLIMITED,
	RETR_FAILURE
};

#ifdef ENABLE_DBM
static int
dbm_retrieve_quota (char *name, mu_off_t *quota)
{
  mu_dbm_file_t db;
  struct mu_dbm_datum  named, contentd;
  char buffer[64];
  int unlimited = 0;
  int rc;

  if (!quotadbname)
    return RETR_FAILURE;

  rc = mu_dbm_create (quotadbname, &db, MU_FILE_SAFETY_ALL);
  if (rc)
    {
      mu_error (_("unable to create quota db"));
      return RETR_FAILURE;
    }

  rc = mu_dbm_safety_check (db);
  if (rc && rc != ENOENT)
    {
      mu_error (_("quota db fails safety check: %s"),
		mu_strerror (rc));
      mu_dbm_destroy (&db);
      return RETR_FAILURE;
    }

  rc = mu_dbm_open (db, MU_STREAM_READ, 0600);
  if (rc)
    {
      mu_error (_("cannot open file %s: %s"),
		quotadbname, mu_strerror (rc));
      mu_dbm_destroy (&db);
      return RETR_FAILURE;
    }
  
  memset (&named, 0, sizeof named);
  memset (&contentd, 0, sizeof contentd);
  named.mu_dptr = name;
  named.mu_dsize = strlen (name);
  rc = mu_dbm_fetch (db, &named, &contentd);
  if (rc)
    {
      /* User not in database, try default quota */
      mu_dbm_datum_free (&named);
      named.mu_dptr = "DEFAULT";
      named.mu_dsize = strlen (named.mu_dptr);
      rc = mu_dbm_fetch (db, &named, &contentd);
      if (rc == MU_ERR_NOENT)
	{
	  mu_dbm_destroy (&db);
	  return RETR_FAILURE;
	}
      else if (rc)
	{
	  mu_error (_("can't fetch data: %s"), mu_dbm_strerror (db));
	  mu_dbm_destroy (&db);
	  return RETR_FAILURE;
	}
    }

  if (mu_c_strncasecmp("none", contentd.mu_dptr, contentd.mu_dsize) == 0) 
      unlimited = 1;
  else if (contentd.mu_dsize > sizeof (buffer) - 1)
    {
      mu_error (ngettext ("mailbox quota for `%s' is too big: %lu digit",
			  "mailbox quota for `%s' is too big: %lu digits",
			  contentd.mu_dsize),
		name, (unsigned long) contentd.mu_dsize);
      *quota = groupquota;
    }
  else
    {
      char *p;
		
      strncpy (buffer, contentd.mu_dptr, contentd.mu_dsize);
      buffer[contentd.mu_dsize] = 0;
      
      *quota = strtoul (buffer, &p, 0);
      if (get_quota (quota, buffer))
	*quota = groupquota;
    }
  
  mu_dbm_datum_free (&contentd);
  mu_dbm_destroy (&db);
  
  return unlimited ? RETR_UNLIMITED : RETR_OK;
}

# define default_retrieve_quota dbm_retrieve_quota
#else
static int
default_retrieve_quota (char *name, mu_off_t *quota)
{
  mu_error (_("no quota retrieving mechanism"));
  return RETR_FAILURE;
}
#endif

#ifdef USE_SQL
#include <mailutils/sql.h>
#include <libmu_auth/sql.h>

static int
sql_retrieve_quota (char *name, mu_off_t *quota)
{
  mu_sql_connection_t conn;
  char *query_str;
  int rc, status;
  char *tmp;
  size_t n;
  
  query_str = mu_sql_expand_query (quota_query, name);
  if (!query_str)
    return RETR_FAILURE;

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
      mu_error ("%s. SQL error: %s",
		mu_strerror (status), mu_sql_strerror (conn));
      mu_sql_connection_destroy (&conn);
      free (query_str);
      return RETR_FAILURE;
    }

  status = mu_sql_connect (conn);

  if (status)
    {
      mu_error ("%s. SQL error: %s",
		mu_strerror (status), mu_sql_strerror (conn));
      mu_sql_connection_destroy (&conn);
      free (query_str);
      return  RETR_FAILURE;
    }
  
  status = mu_sql_query (conn, query_str);
  free (query_str);
  
  if (status)
    {
      mu_error (_("SQL query failed: %s"),
		(status == MU_ERR_SQL) ?  mu_sql_strerror (conn) :
	 	                          mu_strerror (status));
      mu_sql_connection_destroy (&conn);
      return  RETR_FAILURE;
    }

  status = mu_sql_store_result (conn);

  if (status)
    {
      mu_error (_("cannot store SQL result: %s"),
		(status == MU_ERR_SQL) ?  mu_sql_strerror (conn) :
	 	                          mu_strerror (status));
      mu_sql_connection_destroy (&conn);
      return RETR_FAILURE;
    }

  mu_sql_num_tuples (conn, &n);
  if (n == 0)
    {
      rc = RETR_FAILURE;
    }
  else
    {
      rc = RETR_OK;
      tmp = NULL;
      status = mu_sql_get_column (conn, 0, 0, &tmp);
      if (status)
	{
	  mu_error (_("cannot retrieve mailbox quota from SQL: %s"),
		    (status == MU_ERR_SQL) ?  mu_sql_strerror (conn) :
		    mu_strerror (status));
	  rc = RETR_FAILURE;
	}
      else if (tmp == NULL || tmp[0] == 0 || mu_c_strcasecmp (tmp, "none") == 0)
	rc = RETR_UNLIMITED;
      else if (get_quota (quota, tmp)) 
	{
	  *quota = groupquota;
	  rc = RETR_OK;
	}
    }
  
  mu_sql_release_result (conn);
  mu_sql_disconnect (conn);
  mu_sql_connection_destroy (&conn);
  return rc;
}
#endif

static int
retrieve_quota (struct mu_auth_data *auth, mu_off_t *quota)
{
  if (MU_HAS_QUOTA (auth))
    {
      if (auth->quota == 0)
	return RETR_UNLIMITED;
      *quota = auth->quota;
      return RETR_OK;
    }
  
#ifdef USE_SQL
  if (quota_query)
    return sql_retrieve_quota (auth->name, quota);
#endif
  return default_retrieve_quota (auth->name, quota);
}

int
mda_check_quota (struct mu_auth_data *auth, mu_off_t size, mu_off_t *rest)
{
  mu_off_t quota;

  if (!auth)
    return MQUOTA_OK;
  
  switch (retrieve_quota (auth, &quota))
    {
    case RETR_FAILURE:
      return DEFRETVAL;
      
    case RETR_UNLIMITED:
      return MQUOTA_UNLIMITED;

    case RETR_OK:
      if (quota < size)  /* Mailbox full */
	return MQUOTA_EXCEEDED;
	
      if (rest)
	*rest = quota - size;
    }
  
  return MQUOTA_OK;
}
