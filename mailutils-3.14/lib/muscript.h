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

#include <mailutils/sieve.h>
#include <mailutils/diag.h>
#include <mailutils/errno.h>
#include <mailutils/error.h>


typedef struct mu_script_fun *mu_script_t;
typedef struct mu_script_descr *mu_script_descr_t;

mu_script_t mu_script_lang_handler (const char *lang);
mu_script_t mu_script_suffix_handler (const char *name);

int mu_script_init (mu_script_t scr, const char *name, const char **env,
		    mu_script_descr_t *);
int mu_script_done (mu_script_t, mu_script_descr_t);
int mu_script_process_msg (mu_script_t, mu_script_descr_t, mu_message_t msg);
void mu_script_log_enable (mu_script_t scr, mu_script_descr_t descr,
			   const char *name, const char *hdr);

int mu_script_debug_flags (const char *arg, char **endp);

extern int mu_script_debug_guile;
extern int mu_script_debug_sieve;
extern int mu_script_sieve_log;


