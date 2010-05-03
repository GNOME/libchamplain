#define NO_IMPORT_PYGOBJECT

#include <pygobject.h>
#include <champlain/champlain.h>
#include <champlain/champlain-memphis.h>
#include "pychamplainmemphis.h" 

static void
sink_champlain_map_data_source (GObject *object)
{
    if (g_object_is_floating (object)) {
        g_object_ref_sink (object);
    }
}

extern PyMethodDef champlainmemphis_functions[];
DL_EXPORT(void) initchamplainmemphis (void);

DL_EXPORT(void)
initchamplainmemphis (void)
{
    PyObject *m, *d;

    init_pygobject ();

    pygobject_register_sinkfunc (CHAMPLAIN_TYPE_MAP_DATA_SOURCE, sink_champlain_map_data_source);

    m = Py_InitModule ("champlainmemphis", champlainmemphis_functions);
    d = PyModule_GetDict (m);

    champlainmemphis_register_classes (d);

    if (PyErr_Occurred ()) {
        PyErr_Print();
        Py_FatalError ("can't initialise module champlainmemphis");
    }
}
