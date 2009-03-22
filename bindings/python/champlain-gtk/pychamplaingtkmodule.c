#define NO_IMPORT_PYGOBJECT

#include <pygobject.h>
#include "../champlain/pychamplain.h"

DL_EXPORT(void) initchamplaingtk(void);
extern PyMethodDef champlain_functions[];

DL_EXPORT(void)
initchamplaingtk(void)
{
	PyObject *m, *d;

	init_pygobject ();
	
	m = Py_InitModule ("champlaingtk", champlain_functions);
	d = PyModule_GetDict (m);
	
	champlain_register_classes (d);
	
	if (PyErr_Occurred ()) {
		PyErr_Print();
		Py_FatalError ("can't initialise module champlaingtk");
	}
}

