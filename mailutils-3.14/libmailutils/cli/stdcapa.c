/* stdcapa.c -- Standard CLI capabilities for GNU Mailutils
   Copyright (C) 2016-2022 Free Software Foundation, Inc.

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
#include <stdlib.h>
#include <mailutils/cli.h>
#include <mailutils/cfg.h>
#include <mailutils/nls.h>
#include <mailutils/syslog.h>
#include <mailutils/stdstream.h>
#include <mailutils/mailer.h>
#include <mailutils/errno.h>
#include <mailutils/mailbox.h>
#include <mailutils/registrar.h>
#include <mailutils/locker.h>
#include <mailutils/mu_auth.h>
#include <mailutils/url.h>
#include <mailutils/kwd.h>

/* *************************************************************************
 * Logging section
 * ************************************************************************* */
static void
cli_log_facility (struct mu_parseopt *po, struct mu_option *opt,
		  char const *arg)
{
  if (mu_string_to_syslog_facility (arg, &mu_log_facility))
    mu_parseopt_error (po, _("unknown syslog facility `%s'"), arg);
}

static int
cb_facility (void *data, mu_config_value_t *val)
{
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;

  if (mu_string_to_syslog_facility (val->v.string, &mu_log_facility))
    {
      mu_error (_("unknown syslog facility `%s'"), val->v.string);
      return 1;
    }
   return 0;
}

static int
cb_severity (void *data, mu_config_value_t *val)
{
  unsigned n;

  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  if (mu_severity_from_string (val->v.string, &n))
    {
      mu_error (_("unknown severity `%s'"), val->v.string);
      return 1;
    }
  mu_log_severity_threshold = n;
  return 0;
}

static struct mu_cfg_param logging_cfg[] = {
  { "syslog", mu_c_bool, &mu_log_syslog, 0, NULL,
    N_("Send diagnostics to syslog.") },
  { "print-severity", mu_c_bool, &mu_log_print_severity, 0, NULL,
    N_("Print message severity levels.") },
  { "severity", mu_cfg_callback, NULL, 0, cb_severity,
    N_("Output only messages with a severity equal to or greater than "
       "this one."),
    /* TRANSLATORS: Translate only arg:, rest are keywords */
    N_("arg: debug|info|notice|warning|error|crit|alert|emerg")},
  { "facility", mu_cfg_callback, NULL, 0, cb_facility,
    N_("Set syslog facility. Arg is one of the following: user, daemon, "
       "auth, authpriv, mail, cron, local0 through local7 (case-insensitive), "
       "or a facility number."),
    /* TRANSLATORS: Translate only arg: and <number>, rest are keywords */
    N_("arg: auth|authpriv|mail|local0-local7|<number>") },
  { "session-id", mu_c_bool, &mu_log_session_id, 0, NULL,
    N_("Log session ID") },
  { "tag", mu_c_string, &mu_log_tag, 0, NULL,
    N_("Tag syslog messages with this string.") },
  { NULL }
};

static struct mu_option logging_option[] = {
  { "log-facility", 0, N_("FACILITY"), MU_OPTION_DEFAULT,
    N_("output logs to syslog FACILITY"),
    mu_c_int, &mu_log_facility, cli_log_facility },
  MU_OPTION_END
};

static void
logging_commit (void *unused)
{
  if (mu_log_syslog >= 0)
    mu_stdstream_strerr_setup (mu_log_syslog ?
			       MU_STRERR_SYSLOG : MU_STRERR_STDERR);
}

/* *************************************************************************
 * Mailer
 * ************************************************************************* */
static void
cli_mailer (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  int rc = mu_mailer_set_url_default (arg);
  if (rc != 0)
    mu_parseopt_error (po, _("invalid mailer URL `%s': %s"),
		       arg, mu_strerror (rc));
}

static struct mu_option mailer_option[] = {
  { "mailer", 'M', N_("MAILER"), MU_OPTION_DEFAULT,
    N_("use specified URL as the default mailer"),
    mu_c_string, NULL, cli_mailer },
  MU_OPTION_END
};

static int
cb_mailer (void *data, mu_config_value_t *val)
{
  int rc;

  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  rc = mu_mailer_set_url_default (val->v.string);
  if (rc != 0)
    mu_error (_("%s: invalid mailer URL: %s"),
		val->v.string, mu_strerror (rc));
  return rc;
}

static struct mu_cfg_param mailer_cfg[] = {
  { "url", mu_cfg_callback, NULL, 0, cb_mailer,
    N_("Use this URL as the default mailer"),
    N_("url: string") },
  { NULL }
};

/* *************************************************************************
 * Debugging
 * ************************************************************************* */
static void
cli_debug_level (struct mu_parseopt *po, struct mu_option *opt,
		 char const *arg)
{
  mu_debug_clear_all ();
  mu_debug_parse_spec (arg);
  /* FIXME: Error handling */
}

static struct mu_option debug_option[] = {
  MU_OPTION_GROUP (N_("Global debugging settings")),
  { "debug-level", 0, N_("LEVEL"), MU_OPTION_DEFAULT,
    N_("set Mailutils debugging level"),
    mu_c_string, NULL, cli_debug_level },
  { "debug-line-info", 0, NULL,    MU_OPTION_DEFAULT,
    N_("show source info with debugging messages"),
    mu_c_bool, &mu_debug_line_info },
  MU_OPTION_END
};

static int
cb_debug_level (void *data, mu_config_value_t *val)
{
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  mu_debug_parse_spec (val->v.string);
  return 0;
}

static struct mu_cfg_param debug_cfg[] = {
  { "level", mu_cfg_callback, NULL, 0, &cb_debug_level,
    N_("Set Mailutils debugging level. Argument is a semicolon-separated list "
       "of debugging specifications. A simplified specification syntax is:\n"
       "   [!]<category: string>[.<level: string>,<level: string>...]\n"
       "For details, please see the section 3.3 \"Debugging\" of the GNU Mailutils\n"
       "manual, or visit <http://mailutils.org/wiki/Debug_level>."),
    N_("arg: string") },
  { "line-info", mu_c_bool, &mu_debug_line_info, 0, NULL,
    N_("Prefix debug messages with Mailutils source locations.") },
  { NULL }
};

/* ************************************************************************* *
 * Mailbox                                                                   *
 * ************************************************************************* */

static int
cb_mail_spool (void *data, mu_config_value_t *val)
{
  int rc;
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  rc = mu_set_mail_directory (val->v.string);
  if (rc)
    mu_error (_("cannot set mail directory name to `%s': %s"),
	      val->v.string, mu_strerror (rc));
  return rc;
}

static int
cb_mailbox_pattern (void *data, mu_config_value_t *val)
{
  int rc;
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;

  rc = mu_set_mailbox_pattern (val->v.string);
  if (rc)
    mu_error (_("cannot set mailbox pattern to `%s': %s"),
	      val->v.string, mu_strerror (rc));
  return rc;
}

static int
cb_mailbox_type (void *data, mu_config_value_t *val)
{
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  if (mu_registrar_set_default_scheme (val->v.string))
    mu_error (_("invalid mailbox type: %s"), val->v.string);
  return 0;
}

static int
cb_folder (void *data, mu_config_value_t *val)
{
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  mu_set_folder_directory (val->v.string);
  return 0;
}

static int
cb_autodetect_accuracy (void *data, mu_config_value_t *val)
{
  int v;
  char *errmsg;

  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  if (strcmp (val->v.string, "auto") == 0)
    v = MU_AUTODETECT_ACCURACY_AUTO;
  else if (strcmp (val->v.string, "fast") == 0)
    v = MU_AUTODETECT_ACCURACY_FAST;
  else if (strcmp (val->v.string, "minimal") == 0
	   || strcmp (val->v.string, "default") == 0)
    v = MU_AUTODETECT_ACCURACY_DEFAULT;
  else
    {
      int rc = mu_str_to_c (val->v.string, mu_c_int, &v, &errmsg);
      if (rc)
	{
	  mu_error (_("conversion failed: %s"),
		    errmsg ? errmsg : mu_strerror (rc));
	  free (errmsg);
	}
      else
	mu_set_autodetect_accuracy (v);
    }
  return 0;
}

static struct mu_cfg_param mailbox_cfg[] = {
  { "mail-spool", mu_cfg_callback, NULL, 0, cb_mail_spool,
    N_("Use specified URL as a mailspool directory."),
    N_("url: string") },
  { "mailbox-pattern", mu_cfg_callback, NULL, 0, cb_mailbox_pattern,
    N_("Create mailbox URL using <pattern>."),
    N_("pattern: string") },
  { "mailbox-type", mu_cfg_callback, NULL, 0, cb_mailbox_type,
    N_("Default mailbox type."),
    N_("protocol: string") },
  { "folder", mu_cfg_callback, NULL, 0, cb_folder,
    N_("Default user mail folder"),
    N_("dir: string") },
  { "autodetect-accuracy", mu_cfg_callback, NULL, 0, cb_autodetect_accuracy,
    N_("Accuracy level of mailbox format autodetection. Argument is either a"
       " decimal number or any of the following constants:\n"
       "  auto    - set accuracy level from the environment variable\n"
       "            MU_AUTODETECT_ACCURACY (default)\n"
       "  fast    - do only a rough estimation of the mailbox format: fastest,\n"
       "            but possibly inaccurate\n"
       "  minimal - good balance between speed and accuracy"),
    N_("n: number") },
  { NULL }
};

/* ************************************************************************* *
 * Locking                                                                   *
 * ************************************************************************* */
static int
cb_locker_flags (void *data, mu_config_value_t *val)
{
  char const *s;
  static struct mu_kwd flag_tab[] = {
    { "type external",   'E' },
    { "retry-count",     'R' },
    { "expire-timeout",  'T' },
    { "pid-check",       'P' },
    { NULL }
  };
      
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;

  for (s = val->v.string; *s; s++)
    {
      char const *kw;
      if (mu_kwd_xlat_tok (flag_tab, *s, &kw))
	{
	  mu_error (_("invalid lock flag `%c'"), *s);
	}
      else if (*s == 'P')
	{
	  /* TRANSLATORS: %c is replaced with the flag letter, and %s - with
	     the corresponding keyword. */
	  mu_diag_output (MU_DIAG_WARNING,
			  _("applying legacy flag %c, use %s instead"),
			  *s, kw);
	  mu_locker_defaults.flags |= MU_LOCKER_FLAG_CHECK_PID;
	}
      else
	{
	  /* TRANSLATORS: %c is replaced with the flag letter, and %s - with
	     the corresponding keyword. */
	  mu_diag_output (MU_DIAG_WARNING,
			  _("ignoring legacy flag %c, use %s instead"),
			  *s, kw);
	}
    }
  return 0;
}

static int
cb_locker_retry_sleep (void *data, mu_config_value_t *val)
{
  int rc;
  time_t t;
  char *errmsg;

  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  rc = mu_str_to_c (val->v.string, mu_c_time, &t, &errmsg);
  if (rc)
    {
      mu_error (_("conversion failed: %s"), errmsg ? errmsg :
		mu_strerror (rc));
      free (errmsg);
    }
  else
    {
      mu_locker_defaults.flags |= MU_LOCKER_FLAG_RETRY;
      mu_locker_defaults.retry_sleep = t;
    }
  return 0;
}

static int
cb_locker_retry_timeout (void *data, mu_config_value_t *val)
{
  mu_diag_output (MU_DIAG_WARNING,
		  _("%s is deprecated, please use %s instead"),
		  "retry-timeout", "retry-sleep");
  return cb_locker_retry_sleep (data, val);
}

static int
cb_locker_retry_count (void *data, mu_config_value_t *val)
{
  int rc;
  size_t n;
  char *errmsg;

  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  rc = mu_str_to_c (val->v.string, mu_c_size, &n, &errmsg);
  if (rc)
    {
      mu_error (_("conversion failed: %s"), errmsg ? errmsg :
		mu_strerror (rc));
      free (errmsg);
    }
  else if (n == 0)
    mu_locker_defaults.flags &= ~MU_LOCKER_FLAG_RETRY;
  else
    {
      mu_locker_defaults.flags |= MU_LOCKER_FLAG_RETRY;
      mu_locker_defaults.retry_count = n;
    }
  return 0;
}

static int
cb_locker_expire_timeout (void *data, mu_config_value_t *val)
{
  int rc;
  time_t t;
  char *errmsg;

  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  rc = mu_str_to_c (val->v.string, mu_c_time, &t, &errmsg);
  if (rc)
    {
      mu_error (_("conversion failed: %s"), errmsg ? errmsg :
		mu_strerror (rc));
      free (errmsg);
    }
  else if (t == 0)
    mu_locker_defaults.flags &= ~MU_LOCKER_FLAG_EXPIRE_TIME;
  else
    {
      mu_locker_defaults.flags |= MU_LOCKER_FLAG_EXPIRE_TIME;
      mu_locker_defaults.expire_time = t;
    }
  return 0;
}

static int
cb_locker_type (void *data, mu_config_value_t *val)
{
  int t;
  static struct mu_kwd ltab[] = {
    { "dotlock",  MU_LOCKER_TYPE_DOTLOCK },
    { "default",  MU_LOCKER_TYPE_DEFAULT },
    { "external", MU_LOCKER_TYPE_EXTERNAL },
    { "kernel",   MU_LOCKER_TYPE_KERNEL },
    { "null",     MU_LOCKER_TYPE_NULL },
    { NULL }
  };

  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;

  if (mu_kwd_xlat_name (ltab, val->v.string, &t))
    {
      mu_error (_("unrecognized locker type: %s"), val->v.string);
      return 1;
    }

  free (mu_locker_defaults.ext_locker);
  mu_locker_defaults.ext_locker = NULL;

  mu_locker_defaults.type = t;
  mu_locker_defaults.flags |= MU_LOCKER_FLAG_TYPE;
      
  return 0;
}

static int
cb_locker_external (void *data, mu_config_value_t *val)
{
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;

  free (mu_locker_defaults.ext_locker);
  mu_locker_defaults.flags |= MU_LOCKER_FLAG_EXT_LOCKER;
  mu_locker_defaults.ext_locker = strdup (val->v.string);

  return 0;
}

static int
cb_locker_pid_check (void *data, mu_config_value_t *val)
{
  int t;
  
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  if (mu_str_to_c (val->v.string, mu_c_bool, &t, NULL))
    {
      mu_error ("%s", _("not a boolean"));
      return 1;
    }
  if (t)
    mu_locker_defaults.flags |= MU_LOCKER_FLAG_CHECK_PID;
  else
    mu_locker_defaults.flags &= ~MU_LOCKER_FLAG_CHECK_PID;
  return 0;
}
  
static struct mu_cfg_param locking_cfg[] = {
  { "type", mu_cfg_callback, NULL, 0, cb_locker_type,
    N_("Set locker type."),
    N_("type: default | dotlock | external | kernel | null") },
  { "retry-count", mu_cfg_callback, NULL, 0, cb_locker_retry_count,
    N_("Set the maximum number of times to retry acquiring the lock."),
    N_("arg: integer") },
  { "retry-sleep", mu_cfg_callback, NULL, 0, cb_locker_retry_sleep,
    N_("Set the delay between two successive locking attempts."),
    N_("arg: interval")},
  { "retry-timeout", mu_cfg_callback, NULL, 0, cb_locker_retry_timeout,
    N_("Deprecated alias of retry-sleep.  Retained for backward compatibility."),
    N_("arg: interval")},
  { "expire-timeout", mu_cfg_callback, NULL, 0, cb_locker_expire_timeout,
    N_("Expire locks older than this amount of time."),
    N_("arg: interval")},
  { "external-locker", mu_cfg_callback, NULL, 0, cb_locker_external,
    N_("Use external locker program."),
    N_("prog: string") },
  { "pid-check", mu_cfg_callback, NULL, 0, cb_locker_pid_check,
    N_("Check if PID of the lock owner is active."),
    N_("arg: bool") },
  { "flags", mu_cfg_callback, NULL, 0, cb_locker_flags,
    N_("Deprecated.  Retained for backward compatibility."),
    N_("arg: string") },
  { NULL, }
};

/* ************************************************************************* *
 * Address                                                                   *
 * ************************************************************************* */
static int
cb_email_addr (void *data, mu_config_value_t *val)
{
  int rc;

  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;

  rc = mu_set_user_email (val->v.string);
  if (rc)
    mu_error (_("invalid email address `%s': %s"),
	      val->v.string, mu_strerror (rc));
  return 0;
}

static int
cb_email_domain (void *data, mu_config_value_t *val)
{
  int rc;

  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;

  rc = mu_set_user_email_domain (val->v.string);
  if (rc)
    mu_error (_("invalid email domain `%s': %s"),
	      val->v.string, mu_strerror (rc));
  return 0;
}

static struct mu_cfg_param address_cfg[] = {
  { "email-addr", mu_cfg_callback, NULL, 0, cb_email_addr,
    N_("Set the current user email address (default is "
       "loginname@defaultdomain)."),
    N_("email: address") },
  { "email-domain", mu_cfg_callback, NULL, 0, cb_email_domain,
    N_("Set e-mail domain for unqualified user names (default is this host)"),
    N_("domain: string") },
  { NULL }
};

/* ************************************************************************* *
 * Authentication & Authorization                                            *
 * ************************************************************************* */
static int
cb_authentication (void *data, mu_config_value_t *val)
{
  if (val->type == MU_CFG_STRING)
    {
      if (strcmp (val->v.string, "clear") == 0)
	mu_authentication_clear_list ();
      else
	/*FIXME: use err for error reporting*/
	mu_authentication_add_module_list (val->v.string);
    }
  else if (val->type == MU_CFG_LIST)
    {
      int i;
      for (i = 0; i < val->v.arg.c; i++)
	{
	  if (mu_cfg_assert_value_type (&val->v.arg.v[i], MU_CFG_STRING))
	    return 1;
	  if (strcmp (val->v.arg.v[i].v.string, "clear") == 0)
	    mu_authentication_clear_list ();
	  else
	    mu_authentication_add_module (val->v.arg.v[i].v.string);
	}
    }
  else
    {
      mu_error (_("expected string value"));
      return 1;
    }
  return 0;
}

static int
cb_authorization (void *data, mu_config_value_t *val)
{
  if (val->type == MU_CFG_STRING)
    {
      if (strcmp (val->v.string, "clear") == 0)
	mu_authorization_clear_list ();
      else
	/*FIXME: use err for error reporting*/
	mu_authorization_add_module_list (val->v.string);
    }
  else if (val->type == MU_CFG_LIST)
    {
      int i;
      for (i = 0; i < val->v.arg.c; i++)
	{
	  if (mu_cfg_assert_value_type (&val->v.arg.v[i], MU_CFG_STRING))
	    return 1;
	  if (strcmp (val->v.arg.v[i].v.string, "clear") == 0)
	    mu_authorization_clear_list ();
	  else
	    mu_authorization_add_module (val->v.arg.v[i].v.string);
	}
    }
  else
    {
      mu_error (_("expected string value"));
      return 1;
    }
  return 0;
}

static struct mu_cfg_param mu_auth_param[] = {
  { "authentication", mu_cfg_callback, NULL, 0, cb_authentication,
    /* FIXME: The description is incomplete. MU-list is also allowed as
       argument */
    N_("Set a list of modules for authentication. Modlist is a "
       "colon-separated list of module names or a word `clear' to "
       "clear the previously set up values."),
    N_("modlist") },
  { "authorization", mu_cfg_callback, NULL, 0, cb_authorization,
    N_("Set a list of modules for authorization. Modlist is a "
       "colon-separated list of module names or a word `clear' to "
       "clear the previously set up values."),
    N_("modlist") },
  { NULL }
};

int
mu_auth_section_parser
   (enum mu_cfg_section_stage stage, const mu_cfg_node_t *node,
    const char *section_label, void **section_data, void *call_data,
    mu_cfg_tree_t *tree)
{
  switch (stage)
    {
    case mu_cfg_section_start:
      break;

    case mu_cfg_section_end:
      mu_auth_finish_setup ();
    }
  return 0;
}

/* ************************************************************************* *
 * Registry of standard mailutils' capabilities                              *
 * ************************************************************************* */
struct mu_cli_capa mu_cli_std_capa[] = {
  { "mailutils" }, /* Dummy */
  { "logging", logging_option, logging_cfg, NULL, logging_commit },
  { "mailer",  mailer_option, mailer_cfg, NULL, NULL },
  { "debug", debug_option, debug_cfg, NULL, NULL },
  { "mailbox", NULL, mailbox_cfg, NULL, NULL },
  { "locking", NULL, locking_cfg, NULL, NULL },
  { "address", NULL, address_cfg, NULL, NULL },
  { "auth",    NULL, mu_auth_param, mu_auth_section_parser },

  { NULL }
};

void
mu_cli_capa_init (void)
{
  size_t i;

  for (i = 0; mu_cli_std_capa[i].name; i++)
    mu_cli_capa_register (&mu_cli_std_capa[i]);
}
