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

#define PY_MODULE "message"
#define PY_CSNAME "MessageType"

static PyObject *
_repr (PyObject *self)
{
  char buf[80];
  sprintf (buf, "<" PY_MODULE "." PY_CSNAME " instance at %p>", self);
  return PyUnicode_FromString (buf);
}

static PyTypeObject PyMessageType = {
  .ob_base = { PyObject_HEAD_INIT(NULL) },
  .tp_name = PY_MODULE "." PY_CSNAME,
  .tp_basicsize = sizeof (PyMessage),
  .tp_dealloc = (destructor)_py_dealloc,
  .tp_repr = _repr,
  .tp_str = _repr,
  .tp_flags = Py_TPFLAGS_DEFAULT,
  .tp_doc = "",
};

PyMessage *
PyMessage_NEW ()
{
  return (PyMessage *)PyObject_NEW (PyMessage, &PyMessageType);
}

int
PyMessage_Check (PyObject *x)
{
  return x->ob_type == &PyMessageType;
}

static PyObject *
api_message_create (PyObject *self, PyObject *args)
{
  int status;
  PyMessage *py_msg;

  if (!PyArg_ParseTuple (args, "O!", &PyMessageType, &py_msg))
    return NULL;

  status = mu_message_create (&py_msg->msg, NULL);
  return _ro (PyLong_FromLong (status));
}

static PyObject *
api_message_destroy (PyObject *self, PyObject *args)
{
  PyMessage *py_msg;

  if (!PyArg_ParseTuple (args, "O!", &PyMessageType, &py_msg))
    return NULL;

  mu_message_destroy (&py_msg->msg, NULL);
  return _ro (Py_None);
}

static PyObject *
api_message_is_multipart (PyObject *self, PyObject *args)
{
  int status, ismulti;
  PyMessage *py_msg;

  if (!PyArg_ParseTuple (args, "O!", &PyMessageType, &py_msg))
    return NULL;

  status = mu_message_is_multipart (py_msg->msg, &ismulti);
  return status_object (status, PyBool_FromLong (ismulti));
}

static PyObject *
api_message_size (PyObject *self, PyObject *args)
{
  int status;
  size_t size;
  PyMessage *py_msg;

  if (!PyArg_ParseTuple (args, "O!", &PyMessageType, &py_msg))
    return NULL;

  status = mu_message_size (py_msg->msg, &size);
  return status_object (status, PyLong_FromSize_t (size));
}

static PyObject *
api_message_lines (PyObject *self, PyObject *args)
{
  int status;
  size_t lines;
  PyMessage *py_msg;

  if (!PyArg_ParseTuple (args, "O!", &PyMessageType, &py_msg))
    return NULL;

  status = mu_message_lines (py_msg->msg, &lines);
  return status_object (status, PyLong_FromSize_t (lines));
}

static PyObject *
api_message_get_envelope (PyObject *self, PyObject *args)
{
  int status;
  PyMessage *py_msg;
  PyEnvelope *py_env = PyEnvelope_NEW ();

  if (!PyArg_ParseTuple (args, "O!", &PyMessageType, &py_msg))
    return NULL;

  status = mu_message_get_envelope (py_msg->msg, &py_env->env);

  Py_INCREF (py_env);
  return status_object (status, (PyObject *)py_env);
}

static PyObject *
api_message_get_header (PyObject *self, PyObject *args)
{
  int status;
  PyMessage *py_msg;
  PyHeader *py_hdr = PyHeader_NEW ();

  if (!PyArg_ParseTuple (args, "O!", &PyMessageType, &py_msg))
    return NULL;

  status = mu_message_get_header (py_msg->msg, &py_hdr->hdr);

  Py_INCREF (py_hdr);
  return status_object (status, (PyObject *)py_hdr);
}

static PyObject *
api_message_get_body (PyObject *self, PyObject *args)
{
  int status;
  PyMessage *py_msg;
  PyBody *py_body = PyBody_NEW ();

  if (!PyArg_ParseTuple (args, "O!", &PyMessageType, &py_msg))
    return NULL;

  status = mu_message_get_body (py_msg->msg, &py_body->body);

  Py_INCREF (py_body);
  return status_object (status, (PyObject *)py_body);
}

static PyObject *
api_message_get_attribute (PyObject *self, PyObject *args)
{
  int status;
  PyMessage *py_msg;
  PyAttribute *py_attr = PyAttribute_NEW ();

  if (!PyArg_ParseTuple (args, "O!", &PyMessageType, &py_msg))
    return NULL;

  status = mu_message_get_attribute (py_msg->msg, &py_attr->attr);

  Py_INCREF (py_attr);
  return status_object (status, (PyObject *)py_attr);
}

static PyObject *
api_message_get_num_parts (PyObject *self, PyObject *args)
{
  int status;
  size_t parts;
  PyMessage *py_msg;

  if (!PyArg_ParseTuple (args, "O!", &PyMessageType, &py_msg))
    return NULL;

  status = mu_message_get_num_parts (py_msg->msg, &parts);
  return status_object (status, PyLong_FromSize_t (parts));
}

static PyObject *
api_message_get_part (PyObject *self, PyObject *args)
{
  int status;
  Py_ssize_t npart;
  PyMessage *py_msg;
  PyMessage *py_part = PyMessage_NEW ();

  if (!PyArg_ParseTuple (args, "O!n", &PyMessageType, &py_msg, &npart))
    return NULL;
  ASSERT_INDEX_RANGE (npart, "message part");
  status = mu_message_get_part (py_msg->msg, npart, &py_part->msg);

  Py_INCREF (py_part);
  return status_object (status, (PyObject *)py_part);
}

static PyObject *
api_message_get_uid (PyObject *self, PyObject *args)
{
  int status;
  size_t uid;
  PyMessage *py_msg;

  if (!PyArg_ParseTuple (args, "O!", &PyMessageType, &py_msg))
    return NULL;

  status = mu_message_get_uid (py_msg->msg, &uid);
  return status_object (status, PyLong_FromSize_t (uid));
}

static PyObject *
api_message_get_uidl (PyObject *self, PyObject *args)
{
  int status;
  char buf[512];
  size_t writen;
  PyMessage *py_msg;

  memset (buf, 0, sizeof (buf));

  if (!PyArg_ParseTuple (args, "O!", &PyMessageType, &py_msg))
    return NULL;

  status = mu_message_get_uidl (py_msg->msg, buf, sizeof (buf), &writen);
  return status_object (status, PyUnicode_FromString (buf));
}

static PyObject *
api_message_get_attachment_name (PyObject *self, PyObject *args)
{
  int status;
  char *name = NULL;
  char *charset = NULL;
  char *lang = NULL;
  PyObject *py_ret;
  PyMessage *py_msg;

  if (!PyArg_ParseTuple (args, "O!|z", &PyMessageType, &py_msg, &charset))
    return NULL;

  status = mu_message_aget_decoded_attachment_name (py_msg->msg, charset,
						    &name, &lang);

  py_ret = PyTuple_New (3);
  PyTuple_SetItem (py_ret, 0, PyLong_FromLong (status));
  PyTuple_SetItem (py_ret, 1, PyUnicode_FromString (mu_prstr (name)));
  PyTuple_SetItem (py_ret, 2, lang ? PyUnicode_FromString (lang) : Py_None);
  return _ro (py_ret);
}

static PyObject *
api_message_save_attachment (PyObject *self, PyObject *args)
{
  int status;
  char *filename = NULL;
  PyMessage *py_msg;

  if (!PyArg_ParseTuple (args, "O!|s", &PyMessageType, &py_msg,
			 &filename))
    return NULL;

  if (!strlen (filename))
    filename = NULL;

  status = mu_message_save_attachment (py_msg->msg, filename, NULL);
  return _ro (PyLong_FromLong (status));
}

static PyObject *
api_message_unencapsulate (PyObject *self, PyObject *args)
{
  int status;
  PyMessage *py_msg;
  PyMessage *py_unen = PyMessage_NEW ();

  if (!PyArg_ParseTuple (args, "O!", &PyMessageType, &py_msg))
    return NULL;

  Py_INCREF (py_unen);

  status = mu_message_unencapsulate (py_msg->msg, &py_unen->msg, NULL);
  return status_object (status, (PyObject *)py_unen);
}

static PyMethodDef methods[] = {
  { "create", (PyCFunction) api_message_create, METH_VARARGS,
    "Create message." },

  { "destroy", (PyCFunction) api_message_destroy, METH_VARARGS,
    "The resources allocate for 'msg' are freed." },

  { "is_multipart", (PyCFunction) api_message_is_multipart, METH_VARARGS,
    "" },

  { "size", (PyCFunction) api_message_size, METH_VARARGS,
    "Retrieve 'msg' size." },

  { "lines", (PyCFunction) api_message_lines, METH_VARARGS,
    "Retrieve 'msg' number of lines." },

  { "get_envelope", (PyCFunction) api_message_get_envelope, METH_VARARGS,
    "Retrieve 'msg' envelope." },

  { "get_header", (PyCFunction) api_message_get_header, METH_VARARGS,
    "Retrieve 'msg' header." },

  { "get_body", (PyCFunction) api_message_get_body, METH_VARARGS,
    "Retrieve 'msg' body." },

  { "get_attribute", (PyCFunction) api_message_get_attribute, METH_VARARGS,
    "Retrieve 'msg' attribute." },

  { "get_num_parts", (PyCFunction) api_message_get_num_parts, METH_VARARGS,
    "" },

  { "get_part", (PyCFunction) api_message_get_part, METH_VARARGS,
    "" },

  { "get_uid", (PyCFunction) api_message_get_uid, METH_VARARGS,
    "" },

  { "get_uidl", (PyCFunction) api_message_get_uidl, METH_VARARGS,
    "" },

  { "get_attachment_name", (PyCFunction) api_message_get_attachment_name,
    METH_VARARGS, "" },

  { "save_attachment", (PyCFunction) api_message_save_attachment,
    METH_VARARGS, "" },

  { "unencapsulate", (PyCFunction) api_message_unencapsulate,
    METH_VARARGS, "" },

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
mu_py_init_message (void)
{
  PyMessageType.tp_new = PyType_GenericNew;
  return PyType_Ready (&PyMessageType);
}

void
_mu_py_attach_message (void)
{
  PyObject *m;
  if ((m = _mu_py_attach_module (&moduledef)))
    {
      Py_INCREF (&PyMessageType);
      PyModule_AddObject (m, PY_CSNAME, (PyObject *)&PyMessageType);
    }
}
