/*
 * Copyright (C) 2009 Simon Wenner <simon@wenner.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "champlain-map-data-source.h"

G_DEFINE_TYPE (ChamplainMapDataSource, champlain_map_data_source, G_TYPE_OBJECT)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_MAP_DATA_SOURCE, ChamplainMapDataSourcePrivate))

typedef struct _ChamplainMapDataSourcePrivate ChamplainMapDataSourcePrivate;

struct _ChamplainMapDataSourcePrivate {
  MemphisMap *map_data;
};

static void
champlain_map_data_source_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
champlain_map_data_source_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
champlain_map_data_source_dispose (GObject *object)
{
  G_OBJECT_CLASS (champlain_map_data_source_parent_class)->dispose (object);
}

static void
champlain_map_data_source_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_map_data_source_parent_class)->finalize (object);
}

static void
champlain_map_data_source_real_get_map_data (ChamplainMapDataSource *data_source,
      osmFile *map_data)
{
  g_error ("Should not be reached");
}

static void
champlain_map_data_source_class_init (ChamplainMapDataSourceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainMapDataSourcePrivate));

  object_class->get_property = champlain_map_data_source_get_property;
  object_class->set_property = champlain_map_data_source_set_property;
  object_class->dispose = champlain_map_data_source_dispose;
  object_class->finalize = champlain_map_data_source_finalize;

  klass->get_map_data = champlain_map_data_source_real_get_map_data;
}

static void
champlain_map_data_source_init (ChamplainMapDataSource *self)
{

}

ChamplainMapDataSource*
champlain_map_data_source_new (void)
{
  return g_object_new (CHAMPLAIN_TYPE_MAP_DATA_SOURCE, NULL);
}

MemphisMap*
champlain_map_data_get_map_data (ChamplainMapDataSource *data_source)
{

  return NULL;
}
