#define NO_IMPORT_PYGOBJECT

#include <pygobject.h>

void champlain_register_classes (PyObject *d);
DL_EXPORT(void) initchamplain(void);
extern PyMethodDef champlain_functions[];

DL_EXPORT(void)
initchamplain(void)
{
	PyObject *m, *d;

	init_pygobject ();
	
	m = Py_InitModule ("champlain", champlain_functions);
	d = PyModule_GetDict (m);
	
	champlain_register_classes (d);
	
	if (PyErr_Occurred ()) {
		PyErr_Print();
		Py_FatalError ("can't initialise module champlain");
	}
}

