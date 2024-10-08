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

#define PY_MODULE "header"
#define PY_CSNAME "HeaderType"

static PyObject *
_repr (PyObject *self)
{
  char buf[80];
  sprintf (buf, "<" PY_MODULE "." PY_CSNAME " instance at %p>", self);
  return PyUnicode_FromString (buf);
}

static PyTypeObject PyHeaderType = {
  .ob_base = { PyObject_HEAD_INIT(NULL) },
  .tp_name = PY_MODULE "." PY_CSNAME,
  .tp_basicsize = sizeof (PyHeader),
  .tp_dealloc = (destructor)_py_dealloc,
  .tp_repr = _repr,
  .tp_str = _repr,
  .tp_flags = Py_TPFLAGS_DEFAULT,
  .tp_doc = "",
};

PyHeader *
PyHeader_NEW ()
{
  return (PyHeader *)PyObject_NEW (PyHeader, &PyHeaderType);
}

static PyObject *
api_header_size (PyObject *self, PyObject *args)
{
  int status;
  size_t size;
  PyHeader *py_hdr;

  if (!PyArg_ParseTuple (args, "O!", &PyHeaderType, &py_hdr))
    return NULL;

  status = mu_header_size (py_hdr->hdr, &size);
  return status_object (status, PyLong_FromSize_t (size));
}

static PyObject *
api_header_lines (PyObject *self, PyObject *args)
{
  int status;
  size_t lines;
  PyHeader *py_hdr;

  if (!PyArg_ParseTuple (args, "O!", &PyHeaderType, &py_hdr))
    return NULL;

  status = mu_header_lines (py_hdr->hdr, &lines);
  return status_object (status, PyLong_FromSize_t (lines));
}

static PyObject *
api_header_get_value (PyObject *self, PyObject *args)
{
  int status;
  char *name;
  const char *value = NULL;
  PyHeader *py_hdr;

  if (!PyArg_ParseTuple (args, "O!s", &PyHeaderType, &py_hdr, &name))
    return NULL;

  status = mu_header_sget_value (py_hdr->hdr, name, &value);
  return status_object (status, PyUnicode_FromString (mu_prstr (value)));
}

static PyObject *
api_header_get_value_n (PyObject *self, PyObject *args)
{
  int status;
  Py_ssize_t n;
  char *name;
  const char *value = NULL;
  PyHeader *py_hdr;

  if (!PyArg_ParseTuple (args, "O!sn", &PyHeaderType, &py_hdr, &name, &n))
    return NULL;
  ASSERT_INDEX_RANGE (n, "header");
  status = mu_header_sget_value_n (py_hdr->hdr, name, n, &value);
  return status_object (status, PyUnicode_FromString (mu_prstr (value)));
}

static PyObject *
api_header_set_value (PyObject *self, PyObject *args)
{
  int status, replace = 1;
  char *name, *value;
  PyHeader *py_hdr;

  if (!PyArg_ParseTuple (args, "O!ss|i", &PyHeaderType, &py_hdr, &name,
			 &value, &replace))
    return NULL;

  status = mu_header_set_value (py_hdr->hdr, name, value, replace);
  return _ro (PyLong_FromLong (status));
}

static PyObject *
api_header_get_field_count (PyObject *self, PyObject *args)
{
  int status;
  size_t count = 0;
  PyHeader *py_hdr;

  if (!PyArg_ParseTuple (args, "O!", &PyHeaderType, &py_hdr))
    return NULL;

  status = mu_header_get_field_count (py_hdr->hdr, &count);
  return status_object (status, PyLong_FromSize_t (count));
}

static PyObject *
api_header_get_field_name (PyObject *self, PyObject *args)
{
  int status;
  Py_ssize_t idx;
  const char *name = NULL;
  PyHeader *py_hdr;

  if (!PyArg_ParseTuple (args, "O!n", &PyHeaderType, &py_hdr, &idx))
    return NULL;
  ASSERT_INDEX_RANGE (idx, "header");
  status = mu_header_sget_field_name (py_hdr->hdr, idx, &name);
  return status_object (status, PyUnicode_FromString (mu_prstr (name)));
}

static PyObject *
api_header_get_field_value (PyObject *self, PyObject *args)
{
  int status;
  Py_ssize_t idx;
  const char *value = NULL;
  PyHeader *py_hdr;

  if (!PyArg_ParseTuple (args, "O!n", &PyHeaderType, &py_hdr, &idx))
    return NULL;
  ASSERT_INDEX_RANGE (idx, "header");
  status = mu_header_sget_field_value (py_hdr->hdr, idx, &value);
  return status_object (status, PyUnicode_FromString (mu_prstr (value)));
}

static PyMethodDef methods[] = {
  { "size", (PyCFunction) api_header_size, METH_VARARGS,
    "Retrieve 'header' size." },

  { "lines", (PyCFunction) api_header_lines, METH_VARARGS,
    "Retrieve 'header' number of lines." },

  { "get_value", (PyCFunction) api_header_get_value, METH_VARARGS,
    "Retrieve header field value." },

  { "get_value_n", (PyCFunction) api_header_get_value_n, METH_VARARGS,
    "Retrieve Nth header field value." },

  { "set_value", (PyCFunction) api_header_set_value, METH_VARARGS,
    "Set header field value." },

  { "get_field_count", (PyCFunction) api_header_get_field_count, METH_VARARGS,
    "Retrieve the number of header fields." },

  { "get_field_name", (PyCFunction) api_header_get_field_name, METH_VARARGS,
    "Retrieve header field name by field index 'idx'." },

  { "get_field_value", (PyCFunction) api_header_get_field_value, METH_VARARGS,
    "Retrieve header field value by field index 'idx'." },

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
mu_py_init_header (void)
{
  PyHeaderType.tp_new = PyType_GenericNew;
  return PyType_Ready (&PyHeaderType);
}

void
_mu_py_attach_header (void)
{
  PyObject *m;
  if ((m = _mu_py_attach_module (&moduledef)))
    {
      Py_INCREF (&PyHeaderType);
      PyModule_AddObject (m, PY_CSNAME, (PyObject *)&PyHeaderType);
    }
}
