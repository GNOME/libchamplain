#include "champlain-perl.h"


MODULE = Champlain::MapSource  PACKAGE = Champlain::MapSource  PREFIX = champlain_map_source_


const gchar*
champlain_map_source_get_name (ChamplainMapSource *map_source)


gint
champlain_map_source_get_min_zoom_level (ChamplainMapSource *map_source)


gint
champlain_map_source_get_max_zoom_level (ChamplainMapSource *map_source)


guint
champlain_map_source_get_tile_size (ChamplainMapSource *map_source)


guint
champlain_map_source_get_x (ChamplainMapSource *map_source, gint zoom_level, gdouble longitude)


guint
champlain_map_source_get_y (ChamplainMapSource *map_source, gint zoom_level, gdouble latitude)


gdouble
champlain_map_source_get_longitude (ChamplainMapSource *map_source, gint zoom_level, guint x)


gdouble
champlain_map_source_get_latitude (ChamplainMapSource *map_source, gint zoom_level, guint y)


guint
champlain_map_source_get_row_count (ChamplainMapSource *map_source, gint zoom_level)


guint
champlain_map_source_get_column_count (ChamplainMapSource *map_source, gint zoom_level)


void
champlain_map_source_set_name (ChamplainMapSource *map_source, const gchar *name)


void
champlain_map_source_set_license (ChamplainMapSource *map_source, const gchar *license)


const gchar*
champlain_map_source_get_license (ChamplainMapSource *map_source)


void
champlain_map_source_set_projection (ChamplainMapSource *map_source, ChamplainMapProjection projection)


ChamplainMapProjection
champlain_map_source_get_projection (ChamplainMapSource *map_source)


void
champlain_map_source_set_id (ChamplainMapSource *map_source, const gchar *id)


const gchar*
champlain_map_source_get_id (ChamplainMapSource *map_source)


void
champlain_map_source_set_license_uri (ChamplainMapSource *map_source, const gchar *license_uri)


const gchar*
champlain_map_source_get_license_uri (ChamplainMapSource *map_source)


void
champlain_map_source_fill_tile (ChamplainMapSource *map_source, ChamplainTile *tile)
