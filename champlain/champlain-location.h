
#ifndef __CHAMPLAIN_LOCATION_H__
#define __CHAMPLAIN_LOCATION_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_LOCATION            (champlain_location_get_type ())
#define CHAMPLAIN_LOCATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_LOCATION, ChamplainLocation))
#define CHAMPLAIN_IS_LOCATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_LOCATION))
#define CHAMPLAIN_LOCATION_GET_IFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), CHAMPLAIN_TYPE_LOCATION, ChamplainLocationIface))

typedef struct _ChamplainLocation ChamplainLocation; /* Dummy object */
typedef struct _ChamplainLocationIface ChamplainLocationIface;

struct _ChamplainLocationIface
{
  GTypeInterface parent;

  gdouble (* get_latitude) (ChamplainLocation *location);
  gdouble (* get_longitude) (ChamplainLocation *location);
  void (* set_position) (ChamplainLocation *location,
    gdouble latitude,
    gdouble longitude);
};

GType champlain_location_get_type (void) G_GNUC_CONST;

void champlain_location_set_position (ChamplainLocation *location,
    gdouble latitude,
    gdouble longitude);
gdouble champlain_location_get_latitude (ChamplainLocation *location);
gdouble champlain_location_get_longitude (ChamplainLocation *location);

G_END_DECLS

#endif /* __CHAMPLAIN_LOCATION_H__ */
