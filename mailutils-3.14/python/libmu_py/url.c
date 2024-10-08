/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2009-2022 Free Software Foundation, Inc.

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

#include "libmu_py.h"

#define PY_MODULE "url"
#define PY_CSNAME "UrlType"

static PyObject *
_repr (PyObject *self)
{
  char buf[80];
  sprintf (buf, "<" PY_MODULE "." PY_CSNAME " instance at %p>", self);
  return PyUnicode_FromString (buf);
}

static PyTypeObject PyUrlType = {
  .ob_base = { PyObject_HEAD_INIT(NULL) },
  .tp_name = PY_MODULE "." PY_CSNAME,
  .tp_basicsize = sizeof (PyUrl),
  .tp_dealloc = (destructor)_py_dealloc,
  .tp_repr = _repr,
  .tp_str = _repr,
  .tp_flags = Py_TPFLAGS_DEFAULT,
  .tp_doc = "",
};

PyUrl *
PyUrl_NEW ()
{
  return (PyUrl *)PyObject_NEW (PyUrl, &PyUrlType);
}

static PyObject *
api_url_create (PyObject *self, PyObject *args)
{
  int status;
  char *str;
  PyUrl *py_url;

  if (!PyArg_ParseTuple (args, "O!s", &PyUrlType, &py_url, &str))
    return NULL;

  status = mu_url_create (&py_url->url, str);
  return _ro (PyLong_FromLong (status));
}

static PyObject *
api_url_destroy (PyObject *self, PyObject *args)
{
  PyUrl *py_url;

  if (!PyArg_ParseTuple (args, "O!", &PyUrlType, &py_url))
    return NULL;

  mu_url_destroy (&py_url->url);
  return _ro (Py_None);
}

static PyObject *
api_url_get_port (PyObject *self, PyObject *args)
{
  int status;
  unsigned port;
  PyUrl *py_url;

  if (!PyArg_ParseTuple (args, "O!", &PyUrlType, &py_url))
    return NULL;

  status = mu_url_get_port (py_url->url, &port);
  return status_object (status, PyLong_FromLong ((long)port));
}

static PyObject *
api_url_get_scheme (PyObject *self, PyObject *args)
{
  int status;
  const char *buf = NULL;
  PyUrl *py_url;

  if (!PyArg_ParseTuple (args, "O!", &PyUrlType, &py_url))
    return NULL;

  status = mu_url_sget_scheme (py_url->url, &buf);
  return status_object (status, PyUnicode_FromString (mu_prstr (buf)));
}

static PyObject *
api_url_get_user (PyObject *self, PyObject *args)
{
  int status;
  const char *buf = NULL;
  PyUrl *py_url;

  if (!PyArg_ParseTuple (args, "O!", &PyUrlType, &py_url))
    return NULL;

  status = mu_url_sget_user (py_url->url, &buf);
  return status_object (status, PyUnicode_FromString (mu_prstr (buf)));
}

static PyObject *
api_url_get_secret (PyObject *self, PyObject *args)
{
  int status;
  PyUrl *py_url;
  PySecret *py_secret = PySecret_NEW ();

  if (!PyArg_ParseTuple (args, "O!", &PyUrlType, &py_url))
    return NULL;

  Py_INCREF (py_secret);

  status = mu_url_get_secret (py_url->url, &py_secret->secret);
  return status_object (status, (PyObject *)py_secret);
}

static PyObject *
api_url_get_auth (PyObject *self, PyObject *args)
{
  int status;
  const char *buf = NULL;
  PyUrl *py_url;

  if (!PyArg_ParseTuple (args, "O!", &PyUrlType, &py_url))
    return NULL;

  status = mu_url_sget_auth (py_url->url, &buf);
  return status_object (status, PyUnicode_FromString (mu_prstr (buf)));
}

static PyObject *
api_url_get_host (PyObject *self, PyObject *args)
{
  int status;
  const char *buf = NULL;
  PyUrl *py_url;

  if (!PyArg_ParseTuple (args, "O!", &PyUrlType, &py_url))
    return NULL;

  status = mu_url_sget_host (py_url->url, &buf);
  return status_object (status, PyUnicode_FromString (mu_prstr (buf)));
}

static PyObject *
api_url_get_path (PyObject *self, PyObject *args)
{
  int status;
  const char *buf = NULL;
  PyUrl *py_url;

  if (!PyArg_ParseTuple (args, "O!", &PyUrlType, &py_url))
    return NULL;

  status = mu_url_sget_path (py_url->url, &buf);
  return status_object (status, PyUnicode_FromString (mu_prstr (buf)));
}

static PyObject *
api_url_get_query (PyObject *self, PyObject *args)
{
  int status, i;
  size_t argc;
  char **argv;
  PyObject *py_list;
  PyUrl *py_url;

  if (!PyArg_ParseTuple (args, "O!", &PyUrlType, &py_url))
    return NULL;

  status = mu_url_sget_query (py_url->url, &argc, &argv);

  py_list = PyList_New (0);
  for (i = 0; i < argc; i++)
    PyList_Append (py_list, PyUnicode_FromString (argv[i]));

  return status_object (status, py_list);
}

static PyObject *
api_url_to_string (PyObject *self, PyObject *args)
{
  const char *str;
  PyUrl *py_url;

  if (!PyArg_ParseTuple (args, "O!", &PyUrlType, &py_url))
    return NULL;

  str = mu_url_to_string (py_url->url);
  return _ro (PyUnicode_FromString (mu_prstr (str)));
}

static PyMethodDef methods[] = {
  { "create", (PyCFunction) api_url_create, METH_VARARGS,
    "Create the url data structure, but do not parse it." },

  { "destroy", (PyCFunction) api_url_destroy, METH_VARARGS,
    "Destroy the url and free its resources." },

  { "to_string", (PyCFunction) api_url_to_string, METH_VARARGS,
    "" },

  { "get_port", (PyCFunction) api_url_get_port, METH_VARARGS, "" },
  { "get_scheme", (PyCFunction) api_url_get_scheme, METH_VARARGS, "" },
  { "get_user", (PyCFunction) api_url_get_user, METH_VARARGS, "" },
  { "get_secret", (PyCFunction) api_url_get_secret, METH_VARARGS, "" },
  { "get_auth", (PyCFunction) api_url_get_auth, METH_VARARGS, "" },
  { "get_host", (PyCFunction) api_url_get_host, METH_VARARGS, "" },
  { "get_path", (PyCFunction) api_url_get_path, METH_VARARGS, "" },
  { "get_query", (PyCFunction) api_url_get_query, METH_VARARGS, "" },

  { NULL, NULL, 0, NULL }
};

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  PY_MODULE,
  NULL,
  -1,
  methods
};

int
mu_py_init_url (void)
{
  PyUrlType.tp_new = PyType_GenericNew;
  return PyType_Ready (&PyUrlType);
}

void
_mu_py_attach_url (void)
{
  PyObject *m;
  if ((m = _mu_py_attach_module (&moduledef)))
    {
      Py_INCREF (&PyUrlType);
      PyModule_AddObject (m, PY_CSNAME, (PyObject *)&PyUrlType);
    }
}
