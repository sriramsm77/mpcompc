/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2008-2022 Free Software Foundation, Inc.

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

int mu_switch_to_privs (uid_t uid, gid_t gid, mu_list_t retain_groups);
void mu_set_signals (RETSIGTYPE (*handler) (int signo), int *sigv, int sigc);
const char *mu_strexit (int code);

extern int manlock_mandatory_locking;
extern char *manlock_lock_dir;

int manlock_open_mailbox (mu_mailbox_t *pmbox, const char *mailbox_name,
			  int def, int flags);
int manlock_open_mailbox_from_record (mu_mailbox_t *pmbox, mu_record_t record,
				      const char *mailbox_name, int flags);

int manlock_lock (mu_mailbox_t mbox);
int manlock_touchlock (mu_mailbox_t mbox);
int manlock_unlock (mu_mailbox_t mbox);

void manlock_cfg_init (void);

void unistr_downcase (char const *input, char **output);
int unistr_is_substring (char const *haystack, char const *needle);
int unistr_is_substring_dn (char const *haystack, char const *needle);

int message_body_stream (mu_message_t msg, int unix_header,
			 char const *charset,
			 mu_stream_t *pstr);
extern struct mu_cli_capa mu_cli_capa_mime;

