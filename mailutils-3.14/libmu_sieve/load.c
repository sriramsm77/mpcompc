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
# include <config.h>
#endif  

#ifdef HAVE_LIBLTDL
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <string.h>
#include <sieve-priv.h>
#include <ltdl.h>
#include <mailutils/cctype.h>

typedef int (*sieve_module_init_t) (mu_sieve_machine_t mach);

static int _add_load_dir (void *, void *);

static int
sieve_init_load_path (void)
{
  static int inited = 0;

  if (!inited)
    {
      if (lt_dlinit ())
	return 1;
      if (mu_list_foreach (mu_sieve_library_path_prefix, _add_load_dir, NULL))
	return 1;
#ifdef MU_SIEVE_MODDIR
      if (_add_load_dir (MU_SIEVE_MODDIR, NULL))
	return 1;
#endif
      if (mu_list_foreach (mu_sieve_library_path, _add_load_dir, NULL))
	return 1;
      inited = 1;
    }
  return 0;
}

static lt_dlhandle
load_module (mu_sieve_machine_t mach, const char *name)
{
  lt_dlhandle handle;

  if (sieve_init_load_path ())
    return NULL;

  handle = lt_dlopenext (name);
  if (handle)
    {
      sieve_module_init_t init;
      
      init  = (sieve_module_init_t) lt_dlsym (handle, "init");
      if (init)
	{
	  init (mach);
	  return handle;
	}
      else
	{
	  lt_dlclose (handle);
	  handle = NULL;
	}
    }

  if (!handle)
    {
      mu_sieve_error (mach, "%s: %s", name, lt_dlerror ());
      lt_dlexit ();
    }
  return handle;
}

static void
fix_module_name (char *name)
{
  for (; *name; name++)
    {
      if (mu_isalnum (*name) || *name == '.' || *name == ',')
	continue;
      *name = '-';
    }
}

void *
mu_sieve_load_ext (mu_sieve_machine_t mach, const char *name)
{
  lt_dlhandle handle;
  char *modname;

  modname = strdup (name);
  if (!modname)
    return NULL;
  fix_module_name (modname);
  handle = load_module (mach, modname);
  free (modname);
  return handle;
}

void
mu_sieve_unload_ext (void *data)
{
  if (data)
    lt_dlclose ((lt_dlhandle)data);
}

static int
_add_load_dir (void *item, void *unused)
{
  if (lt_dladdsearchdir (item))
    {
      mu_error (_("can't add dynamic library search directory: %s"),
		lt_dlerror ());
      return MU_ERR_FAILURE;
    }
  return 0;
}

int
mu_i_sv_load_add_dir (mu_sieve_machine_t mach, const char *name)
{
  if (sieve_init_load_path ())
    return 1;
  mu_sieve_machine_add_destructor (mach, (mu_sieve_destructor_t) lt_dlexit, 
                                   NULL);
  return lt_dladdsearchdir (name);
}

#else
#include <sieve-priv.h>

void *
mu_sieve_load_ext (mu_sieve_machine_t mach, const char *name)
{
  errno = ENOSYS;
  return NULL;
}

void
mu_sieve_unload_ext (void *data)
{
}

int
mu_i_sv_load_add_dir (mu_sieve_machine_t mach, const char *name)
{
  errno = ENOSYS;
  return 1;
}

#endif /* HAVE_LIBLTDL */
