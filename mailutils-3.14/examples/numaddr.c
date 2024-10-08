/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2022 Free Software Foundation, Inc.

   GNU Mailutils is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Mailutils is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with GNU Mailutils.  If not, see
   <http://www.gnu.org/licenses/>. */; 

/*  This is an example on how to write extension tests for GNU sieve.
    It provides test "numaddr".

    Syntax:   numaddr [":over" / ":under"] <header-names: string-list>
              <limit: number>

    The "numaddr" test counts Internet addresses in structured headers
    that contain addresses.  It returns true if the total number of
    addresses satisfies the requested relation:

    If the argument is ":over" and the number of addresses is greater than
    the number provided, the test is true; otherwise, it is false.

    If the argument is ":under" and the number of addresses is less than
    the number provided, the test is true; otherwise, it is false.

    If the argument is empty, ":over" is assumed. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif  

#include <stdlib.h>
#include <mailutils/sieve.h>

struct val_ctr {  /* Data passed to the counter function */
  mu_header_t hdr;   /* Headers of the current message */
  size_t limit;   /* Limit for the number of addresses */
  size_t count;   /* Number of addresses counted so far */
};

/* Count addresses in a single header value.

   Input:
     ITEM is the name of the header to scan.
     DATA is a pointer to the val_ctr structure 
   Return value:
     non-zero if the limit on the number of addresses has been reached. */
     
static int
_count_items (void *item, void *data)
{
  char *name = item;
  struct val_ctr *vp = data;
  char *val;
  mu_address_t addr;
  size_t count = 0;
  
  if (mu_header_aget_value (vp->hdr, name, &val))
    return 0;
  if (mu_address_create (&addr, val) == 0)
    {
      mu_address_get_count (addr, &count);
      mu_address_destroy (&addr);
      vp->count += count;
    }
  free (val);
  return vp->count >= vp->limit;
}

/* Handler for the numaddr test */
static int
numaddr_test (mu_sieve_machine_t mach)
{
  mu_sieve_value_t *h;
  struct val_ctr vc;
  int rc;
  
  /* Retrieve required arguments: */
  /* First argument: list of header names */
  h = mu_sieve_get_arg_untyped (mach, 0);
  /* Second argument: Limit on the number of addresses */
  mu_sieve_get_arg (mach, 1, SVT_NUMBER, &vc.limit);

  /* Fill in the val_ctr structure */
  mu_message_get_header (mu_sieve_get_message (mach), &vc.hdr);
  vc.count = 0;

  /* Count the addresses */
  rc = mu_sieve_vlist_do (mach, h, _count_items, &vc);

  /* Here rc >= 1 iff the counted number of addresses is greater or equal
     to vc.limit. If `:under' tag was given we reverse the return value */
  if (mu_sieve_get_tag (mach, "under", SVT_VOID, NULL))
    rc = !rc;

  return rc;
}

/* Syntactic definitions for the numaddr test */

/* Required arguments: */
static mu_sieve_data_type numaddr_req_args[] = {
  SVT_STRING_LIST,
  SVT_NUMBER,
  SVT_VOID
};

/* Tagged arguments: */
static mu_sieve_tag_def_t numaddr_tags[] = {
  { "over", SVT_VOID },
  { "under", SVT_VOID },
  { NULL }
};

static mu_sieve_tag_group_t numaddr_tag_groups[] = {
  { numaddr_tags, NULL },
  { NULL }
};

/* Initialization function. It is the only function exported from this
   module. */
int
SIEVE_EXPORT(numaddr,init) (mu_sieve_machine_t mach)
{
  mu_sieve_register_test (mach, "numaddr", numaddr_test,
			  numaddr_req_args, numaddr_tag_groups, 1);
  return 0;
}
