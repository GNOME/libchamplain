
#include "champlain-location.h"
#include "champlain-private.h"


static void
champlain_location_base_init (gpointer g_iface)
{
  static gboolean initialized = FALSE;

  if (!initialized)
    {
      /**
       * ChamplainMarker:longitude:
       *
       * The longitude coordonate of the map
       *
       * Since: 0.10
       */
      g_object_interface_install_property (g_iface,
          g_param_spec_double ("longitude", "Longitude",
              "The longitude coordonate of the marker",
              -180.0f, 180.0f, 0.0f, CHAMPLAIN_PARAM_READWRITE));

      /**
       * ChamplainMarker:latitude:
       *
       * The latitude coordonate of the map
       *
       * Since: 0.10
       */
      g_object_interface_install_property (g_iface,
          g_param_spec_double ("latitude", "Latitude",
              "The latitude coordonate of the marker",
              -90.0f, 90.0f, 0.0f, CHAMPLAIN_PARAM_READWRITE));

      initialized = TRUE;
    }
}


GType
champlain_location_get_type (void)
{
  static GType type = 0;
  if (type == 0)
    {
      static const GTypeInfo info =
      {
        sizeof (ChamplainLocationIface),
        champlain_location_base_init,          /* base_init */
        NULL,
      };
      type = g_type_register_static (G_TYPE_INTERFACE,
                                     "ChamplainLocation", &info, 0);
    }
  return type;
}


/**
 * champlain_location_set_position:
 * @marker: a #ChamplainMarker
 * @latitude: the longitude to center the map at
 * @longitude: the longitude to center the map at
 *
 * Positions the marker on the map at the coordinates
 *
 * Since: 0.10
 */
void 
champlain_location_set_position (ChamplainLocation *location,
    gdouble latitude,
    gdouble longitude)
{
  CHAMPLAIN_LOCATION_GET_IFACE (location)->set_position (location,
                                                         latitude,
                                                         longitude);
}


/**
 * champlain_location_get_latitude:
 * @marker: a #ChamplainMarker
 *
 * Gets the latitude of the marker.
 *
 * Returns: the latitude of the marker.
 *
 * Since: 0.10
 */
gdouble 
champlain_location_get_latitude (ChamplainLocation *location)
{
  return CHAMPLAIN_LOCATION_GET_IFACE (location)->get_latitude (location);
}


/**
 * champlain_location_get_longitude:
 * @marker: a #ChamplainMarker
 *
 * Gets the longitude of the marker.
 *
 * Returns: the longitude of the marker.
 *
 * Since: 0.10
 */
gdouble 
champlain_location_get_longitude (ChamplainLocation *location)
{
  return CHAMPLAIN_LOCATION_GET_IFACE (location)->get_longitude (location);
}

