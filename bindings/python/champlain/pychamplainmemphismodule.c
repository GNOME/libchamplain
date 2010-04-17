#define NO_IMPORT_PYGOBJECT

#include <pygobject.h>
#include <champlain/champlain.h>
#include <champlain/champlain-memphis.h>
#include "pychamplainmemphis.h" 

extern PyMethodDef champlainmemphis_functions[];
DL_EXPORT(void) initchamplainmemphis (void);

DL_EXPORT(void)
initchamplainmemphis (void)
{
    PyObject *m, *d;

    init_pygobject ();

    m = Py_InitModule ("champlainmemphis", champlainmemphis_functions);
    d = PyModule_GetDict (m);

    champlainmemphis_register_classes (d);

    if (PyErr_Occurred ()) {
        PyErr_Print();
        Py_FatalError ("can't initialise module champlainmemphis");
    }
}
