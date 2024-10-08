/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2009-2022 Free Software Foundation, Inc.

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

#if defined(HAVE_CONFIG_H)
# include <config.h>
#endif
#include <Python.h>
#include "muscript.h"
#include "muscript_priv.h"
#include <string.h>
#include <mailutils/python.h>

static int
python_init (const char *prog, const char **env, mu_script_descr_t *pdescr)
{
  *pdescr = (mu_script_descr_t) strdup (prog);
  if (!*pdescr)
    return errno;
  return 0;
}

static int
python_done (mu_script_descr_t descr)
{
  free (descr);
  return 0;
}

static int
python_proc (mu_script_descr_t descr, mu_message_t msg)
{
  PyMessage *py_msg;
  mu_py_dict dict[2];
  mu_py_script_data data[1];
  char *argv[] = { NULL, NULL };
  char *argv0 = mu_strdup (mu_program_name);
  
  argv[0] = argv0;
  
  mu_py_script_init (1, argv);

  py_msg = PyMessage_NEW ();
  py_msg->msg = msg;
  Py_INCREF (py_msg);

  dict[0].name = "message";
  dict[0].obj  = (PyObject *)py_msg;
  data[0].module_name = mu_program_name;
  data[0].attrs = dict;
  dict[1].name = NULL;

  mu_py_script_run ((char*)descr, data);
  mu_py_script_finish ();

  free (argv0);
  
  return 0;
}

struct mu_script_fun mu_script_python = {
  "python",
  "py\0pyc\0",
  python_init,
  python_done,
  python_proc,
  NULL
};

