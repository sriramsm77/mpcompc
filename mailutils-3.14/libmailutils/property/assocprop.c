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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <mailutils/sys/property.h>
#include <mailutils/errno.h>
#include <mailutils/assoc.h>
#include <mailutils/stream.h>
#include <mailutils/iterator.h>
#include <mailutils/list.h>
#include <stdlib.h>

static void
_assoc_prop_done (struct _mu_property *prop)
{
  mu_assoc_t assoc = prop->_prop_data;
  mu_stream_t str = prop->_prop_init_data;
  mu_assoc_destroy (&assoc);
  mu_stream_destroy (&str);
}

static int
_assoc_prop_getval (struct _mu_property *prop,
		    const char *key, const char **pval)
{
  mu_assoc_t assoc = prop->_prop_data;
  char *item;

  item = mu_assoc_get (assoc, key);
  if (item == NULL)
    return MU_ERR_NOENT;
  if (pval)
    *pval = item;
  return 0;

}

static int
_assoc_prop_setval (struct _mu_property *prop, const char *key,
		       const char *val, int overwrite)
{
  mu_assoc_t assoc = prop->_prop_data;
  char **item;
  int rc;

  rc = mu_assoc_install_ref (assoc, key, &item);
  if (rc == 0)
    {
      *item = strdup (val);
      if (!*item)
	{
	  mu_assoc_remove (assoc, key);
	  return ENOMEM;
	}
    }
  else if (rc == MU_ERR_EXISTS && overwrite)
    {
      char *newval = strdup (val);
      if (!newval)
	return ENOMEM;
      free (*item);
      *item = newval;
    }
  else
    return rc;
  return 0;
}

static int
_assoc_prop_unset (struct _mu_property *prop, const char *key)
{
  mu_assoc_t assoc = prop->_prop_data;

  return mu_assoc_remove (assoc, key);
}

static int
_assoc_prop_clear (struct _mu_property *prop)
{
  mu_assoc_t assoc = prop->_prop_data;
  mu_assoc_clear (assoc);
  return 0;
}


static int
_assoc_prop_getitr (struct _mu_property *prop, mu_iterator_t *pitr)
{
  int rc;
  mu_iterator_t itr;
  
  rc = mu_assoc_get_iterator ((mu_assoc_t)prop->_prop_data, &itr);
  if (rc)
    return rc;
  *pitr = itr;
  return 0;
}


static int
_assoc_prop_fill (struct _mu_property *prop)
{
  int rc;
  mu_stream_t str = prop->_prop_init_data;
  int state = 0;
  char *buf[2] = { NULL, NULL };
  size_t size[2] = { 0, 0 }, n;
  
  if (!str)
    return 0;
  mu_stream_seek (str, 0, MU_SEEK_SET, NULL);
  while ((rc = mu_stream_getdelim (str, &buf[state], &size[state],
				   0, &n)) == 0 &&
	 n > 0)
    {
      if (state == 1)
	_assoc_prop_setval (prop, buf[0], buf[1], 1);
      state = !state;
    }
  free (buf[0]);
  free (buf[1]);
  return rc;
}

static int
_assoc_prop_save (struct _mu_property *prop)
{
  mu_stream_t str = prop->_prop_init_data;
  mu_iterator_t itr;
  int rc;
  mu_off_t off;
  
  if (!str)
    return 0;
  rc = mu_property_get_iterator (prop, &itr);
  if (rc)
    return rc;

  mu_stream_seek (str, 0, MU_SEEK_SET, NULL);
  for (mu_iterator_first (itr); !mu_iterator_is_done (itr);
       mu_iterator_next (itr))
    {
      const char *name, *val;
	  
      mu_iterator_current_kv (itr, (const void **)&name, (void**)&val);
      rc = mu_stream_write (str, name, strlen (name) + 1, NULL);
      if (rc)
	break;
      rc = mu_stream_write (str, val, strlen (val) + 1, NULL);
      if (rc)
	break;
    }      
  mu_iterator_destroy (&itr);
  rc = mu_stream_seek (str, 0, MU_SEEK_CUR, &off);
  if (rc == 0)
    rc = mu_stream_truncate (str, off);
  return rc;
}

int
mu_assoc_property_init (struct _mu_property *prop)
{
  mu_assoc_t assoc;
  int rc;
  
  rc = mu_assoc_create (&assoc, 0);
  if (rc)
    return rc;
  mu_assoc_set_destroy_item (assoc, mu_list_free_item);
  prop->_prop_data = assoc;

  prop->_prop_done = _assoc_prop_done;
  if (prop->_prop_init_data)
    {
      prop->_prop_fill = _assoc_prop_fill;
      prop->_prop_save = _assoc_prop_save;
    }
  else
    {
      prop->_prop_fill = NULL;
      prop->_prop_save = NULL;
    }
  prop->_prop_getval = _assoc_prop_getval;
  prop->_prop_setval = _assoc_prop_setval;
  prop->_prop_unset = _assoc_prop_unset;
  prop->_prop_getitr = _assoc_prop_getitr;
  prop->_prop_clear = _assoc_prop_clear;
  return 0;
}

