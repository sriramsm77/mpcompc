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

#define PY_MODULE "attribute"
#define PY_CSNAME "AttributeType"

static PyObject *
_repr (PyObject *self)
{
  char buf[80];
  sprintf (buf, "<" PY_MODULE "." PY_CSNAME " instance at %p>", self);
  return PyUnicode_FromString (buf);
}

static PyTypeObject PyAttributeType = {
  .ob_base = { PyObject_HEAD_INIT(NULL) },
  .tp_name = PY_MODULE "." PY_CSNAME,
  .tp_basicsize = sizeof (PyAttribute),
  .tp_dealloc = (destructor)_py_dealloc,
  .tp_repr = _repr,
  .tp_str = _repr,
  .tp_flags = Py_TPFLAGS_DEFAULT,
  .tp_doc = "",
};

PyAttribute *
PyAttribute_NEW ()
{
  return (PyAttribute *)PyObject_NEW (PyAttribute, &PyAttributeType);
}

static PyObject *
api_attribute_create (PyObject *self, PyObject *args)
{
  int status;
  PyAttribute *py_attr;

  if (!PyArg_ParseTuple (args, "O!", &PyAttributeType, &py_attr))
    return NULL;

  status = mu_attribute_create (&py_attr->attr, NULL);
  return _ro (PyLong_FromLong (status));
}

static PyObject *
api_attribute_destroy (PyObject *self, PyObject *args)
{
  PyAttribute *py_attr;

  if (!PyArg_ParseTuple (args, "O!", &PyAttributeType, &py_attr))
    return NULL;

  mu_attribute_destroy (&py_attr->attr, NULL);
  return _ro (Py_None);
}

static PyObject *
api_attribute_is_modified (PyObject *self, PyObject *args)
{
  int state;
  PyAttribute *py_attr;

  if (!PyArg_ParseTuple (args, "O!", &PyAttributeType, &py_attr))
    return NULL;

  state = mu_attribute_is_modified (py_attr->attr);
  return _ro (PyBool_FromLong (state));
}

static PyObject *
api_attribute_clear_modified (PyObject *self, PyObject *args)
{
  PyAttribute *py_attr;

  if (!PyArg_ParseTuple (args, "O!", &PyAttributeType, &py_attr))
    return NULL;

  mu_attribute_clear_modified (py_attr->attr);
  return _ro (Py_None);
}

static PyObject *
api_attribute_set_modified (PyObject *self, PyObject *args)
{
  PyAttribute *py_attr;

  if (!PyArg_ParseTuple (args, "O!", &PyAttributeType, &py_attr))
    return NULL;

  mu_attribute_set_modified (py_attr->attr);
  return _ro (Py_None);
}

static PyObject *
api_attribute_get_flags (PyObject *self, PyObject *args)
{
  int status, flags = 0;
  PyAttribute *py_attr;

  if (!PyArg_ParseTuple (args, "O!", &PyAttributeType, &py_attr))
    return NULL;

  status = mu_attribute_get_flags (py_attr->attr, &flags);
  return status_object (status, PyLong_FromLong (flags));
}

static PyObject *
api_attribute_set_flags (PyObject *self, PyObject *args)
{
  int status, flags;
  PyAttribute *py_attr;

  if (!PyArg_ParseTuple (args, "O!i", &PyAttributeType, &py_attr, &flags))
    return NULL;

  status = mu_attribute_set_flags (py_attr->attr, flags);
  return _ro (PyLong_FromLong (status));
}

static PyObject *
api_attribute_unset_flags (PyObject *self, PyObject *args)
{
  int status, flags;
  PyAttribute *py_attr;

  if (!PyArg_ParseTuple (args, "O!i", &PyAttributeType, &py_attr, &flags))
    return NULL;

  status = mu_attribute_unset_flags (py_attr->attr, flags);
  return _ro (PyLong_FromLong (status));
}

static PyObject *
api_attribute_is_userflag (PyObject *self, PyObject *args)
{
  int state, flag;
  PyAttribute *py_attr;

  if (!PyArg_ParseTuple (args, "O!i", &PyAttributeType, &py_attr, &flag))
    return NULL;

  state = mu_attribute_is_userflag (py_attr->attr, flag);
  return _ro (PyBool_FromLong (state));
}

static PyObject *
api_attribute_set_userflag (PyObject *self, PyObject *args)
{
  int status, flag;
  PyAttribute *py_attr;

  if (!PyArg_ParseTuple (args, "O!i", &PyAttributeType, &py_attr, &flag))
    return NULL;

  status = mu_attribute_set_userflag (py_attr->attr, flag);
  return _ro (PyLong_FromLong (status));
}

static PyObject *
api_attribute_unset_userflag (PyObject *self, PyObject *args)
{
  int status, flag;
  PyAttribute *py_attr;

  if (!PyArg_ParseTuple (args, "O!i", &PyAttributeType, &py_attr, &flag))
    return NULL;

  status = mu_attribute_unset_userflag (py_attr->attr, flag);
  return _ro (PyLong_FromLong (status));
}

static PyObject *
api_attribute_to_string (PyObject *self, PyObject *args)
{
  char buf[MU_STATUS_BUF_SIZE];
  size_t na = 0;
  PyAttribute *py_attr;

  memset (buf, 0, sizeof (buf));

  if (!PyArg_ParseTuple (args, "O!", &PyAttributeType, &py_attr))
    return NULL;

  mu_attribute_to_string (py_attr->attr, buf, sizeof (buf), &na);
  return _ro (PyUnicode_FromString (buf));
}

static PyMethodDef methods[] = {
  { "create", (PyCFunction) api_attribute_create, METH_VARARGS,
    "" },

  { "destroy", (PyCFunction) api_attribute_destroy, METH_VARARGS,
    "" },

  { "is_modified", (PyCFunction) api_attribute_is_modified, METH_VARARGS,
    "Return True or False whether attribute has been modified." },

  { "clear_modified", (PyCFunction) api_attribute_clear_modified,
    METH_VARARGS, "" },

  { "set_modified", (PyCFunction) api_attribute_set_modified, METH_VARARGS,
    "" },

  { "get_flags", (PyCFunction) api_attribute_get_flags, METH_VARARGS,
    "" },

  { "set_flags", (PyCFunction) api_attribute_set_flags, METH_VARARGS,
    "" },

  { "unset_flags", (PyCFunction) api_attribute_unset_flags, METH_VARARGS,
    "" },

  { "is_userflag", (PyCFunction) api_attribute_is_userflag, METH_VARARGS,
    "" },

  { "set_userflag", (PyCFunction) api_attribute_set_userflag, METH_VARARGS,
    "" },

  { "unset_userflag", (PyCFunction) api_attribute_unset_userflag,
    METH_VARARGS, "" },

  { "to_string", (PyCFunction) api_attribute_to_string, METH_VARARGS,
    "" },

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
mu_py_init_attribute (void)
{
  PyAttributeType.tp_new = PyType_GenericNew;
  return PyType_Ready (&PyAttributeType);
}

void
_mu_py_attach_attribute (void)
{
  PyObject *m;
  if ((m = _mu_py_attach_module (&moduledef)))
    {
      Py_INCREF (&PyAttributeType);
      PyModule_AddObject (m, PY_CSNAME, (PyObject *)&PyAttributeType);
    }
}
