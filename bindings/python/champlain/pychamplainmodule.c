#define NO_IMPORT_PYGOBJECT

#include <pygobject.h>
#include "pychamplain.h"
#include <champlain/champlain.h>

static void
sink_champlain_map_source (GObject *object)
{
    if (g_object_is_floating (object)) {
        g_object_ref_sink (object);
    }
}

DL_EXPORT(void) initchamplain (void);
extern PyMethodDef champlain_functions[];

DL_EXPORT(void)
initchamplain (void)
{
    PyObject *m, *d;

    init_pygobject ();

    pygobject_register_sinkfunc (CHAMPLAIN_TYPE_MAP_SOURCE, sink_champlain_map_source);

    m = Py_InitModule ("champlain", champlain_functions);
    d = PyModule_GetDict (m);

    champlain_register_classes (d);
    champlain_add_constants(m, "CHAMPLAIN_");

    /* constants */
    PyModule_AddObject(m, "MAP_SOURCE_OSM_MAPNIK", Py_BuildValue("s", 
        CHAMPLAIN_MAP_SOURCE_OSM_MAPNIK));
    PyModule_AddObject(m, "MAP_SOURCE_OSM_OSMARENDER", Py_BuildValue("s", 
        CHAMPLAIN_MAP_SOURCE_OSM_OSMARENDER));
    PyModule_AddObject(m, "MAP_SOURCE_OSM_CYCLE_MAP", Py_BuildValue("s", 
        CHAMPLAIN_MAP_SOURCE_OSM_CYCLE_MAP));
    PyModule_AddObject(m, "MAP_SOURCE_OSM_TRANSPORT_MAP", Py_BuildValue("s", 
        CHAMPLAIN_MAP_SOURCE_OSM_TRANSPORT_MAP));
    PyModule_AddObject(m, "MAP_SOURCE_OAM", Py_BuildValue("s", 
        CHAMPLAIN_MAP_SOURCE_OAM));
    PyModule_AddObject(m, "MAP_SOURCE_MFF_RELIEF", Py_BuildValue("s", 
        CHAMPLAIN_MAP_SOURCE_MFF_RELIEF));

    PyModule_AddObject(m, "MIN_LAT", Py_BuildValue("i", CHAMPLAIN_MIN_LAT));
    PyModule_AddObject(m, "MAX_LAT", Py_BuildValue("i", CHAMPLAIN_MAX_LAT));
    PyModule_AddObject(m, "MIN_LONG", Py_BuildValue("i", CHAMPLAIN_MIN_LONG));
    PyModule_AddObject(m, "MAX_LONG", Py_BuildValue("i", CHAMPLAIN_MAX_LONG));

    if (PyErr_Occurred ()) {
        PyErr_Print();
        Py_FatalError ("can't initialise module champlain");
    }
}
