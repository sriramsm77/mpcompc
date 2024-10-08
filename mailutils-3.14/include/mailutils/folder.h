/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2022 Free Software Foundation, Inc.

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

#ifndef _MAILUTILS_FOLDER_H
# define _MAILUTILS_FOLDER_H

#include <mailutils/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mu_list_response
{
  int type;             /* MU_FOLDER_ATTRIBUTE_.* flags */
  int depth;            /* Item depth within the hierarchy */
  int separator;        /* Directory separator character */
  char *name;           /* Item name */
  mu_record_t format;   /* Associated mailbox format record */
};

typedef int (*mu_folder_match_fp) (const char *, void *, int);
typedef int (*mu_folder_enumerate_fp) (mu_folder_t, struct mu_list_response *,
				       void *data);
  
/* Constructor/destructor and possible types.  */
extern int  mu_folder_create         (mu_folder_t *, const char *);
extern int  mu_folder_create_from_record (mu_folder_t *, mu_url_t url,
					  mu_record_t);
  
extern void mu_folder_destroy        (mu_folder_t *);

extern int  mu_folder_open           (mu_folder_t, int flag);
extern int  mu_folder_close          (mu_folder_t);

extern int  mu_folder_delete         (mu_folder_t, const char *);
extern int  mu_folder_rename         (mu_folder_t, const char *, const char *);
extern int  mu_folder_subscribe      (mu_folder_t, const char *);
extern int  mu_folder_unsubscribe    (mu_folder_t, const char *);
extern int  mu_folder_lsub           (mu_folder_t, const char *, const char *,
				      mu_list_t *);

extern int mu_folder_attach_ticket (mu_folder_t folder);
extern int mu_folder_is_local (mu_folder_t folder);

  /* Scan API */
/* The mu_folder_scanner structure controls the scanning. All of
   members are optional. Unused ones must be initialized to 0.
*/
struct mu_folder_scanner
{
  char const *refname;     /* Reference name */
  void *pattern;           /* Matching pattern */
  int match_flags;         /* Matching flags */
  size_t max_depth;        /* Max. depth to descend
			      (1-based, 0 means 'unlimited') */
  mu_folder_enumerate_fp enumfun; /* Enumeration function */
  void *enumdata;                 /* Data for enumfun */
  mu_list_t records;        /* List of allowed records */
  mu_list_t result;         /* Result list */
};

#define MU_FOLDER_SCANNER_INITIALIZER \
  { NULL, NULL, 0, 0, NULL, NULL, NULL, NULL }
  
int mu_folder_scan (mu_folder_t folder, struct mu_folder_scanner *scn);

/* The following two functions are implemented as alternative entry
   points to mu_folder_scan: */
extern int  mu_folder_list           (mu_folder_t, const char *, void *,
				      size_t, mu_list_t *);
extern int  mu_folder_enumerate      (mu_folder_t, const char *,
				      void *, int, 
				      size_t, mu_list_t *,
				      mu_folder_enumerate_fp, void *);
    
  /* Match function */
extern int mu_folder_set_match (mu_folder_t folder, mu_folder_match_fp pmatch);
extern int mu_folder_get_match (mu_folder_t folder,
				mu_folder_match_fp *pmatch);

  /* Two often used matchers: */
  /* 1. The default: IMAP-style wildcards: */
extern int mu_folder_imap_match (const char *name, void *pattern, int flags);
  /* 2. UNIX-style glob(7) wildcards: */
extern int mu_folder_glob_match (const char *name, void *pattern, int flags);
  
  /* Notifications.  */
extern int  mu_folder_get_observable (mu_folder_t, mu_observable_t *);

/* Authentication.  */
extern int  mu_folder_get_authority  (mu_folder_t, mu_authority_t *);
extern int  mu_folder_set_authority  (mu_folder_t, mu_authority_t);

/* URL.  */
extern int  mu_folder_get_url        (mu_folder_t, mu_url_t *);
extern int  mu_folder_set_url        (mu_folder_t, mu_url_t);

/* Property */
extern int  mu_folder_set_property   (mu_folder_t, mu_property_t);  
extern int  mu_folder_get_property   (mu_folder_t, mu_property_t *);
  
/* FIXME: not implemented */
extern int  mu_folder_decrement      (mu_folder_t);

extern void mu_list_response_free    (void *data);

int _mu_fsfolder_init (mu_folder_t folder);
  
#ifdef __cplusplus
}
#endif

#endif /* _MAILUTILS_FOLDER_H */
