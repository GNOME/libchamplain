#ifndef __PYCHAMPLAIN_H__
#define __PYCHAMPLAIN_H__

void champlain_register_classes (PyObject *d);
void champlain_add_constants(PyObject *module, const gchar *strip_prefix);

#endif
