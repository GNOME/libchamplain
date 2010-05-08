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

/**
 * SECTION:champlain-map-data-source
 * @short_description: Base class for map data sources.
 *
 * ChamplainMapDataSource provides the interface for
 *  #ChamplainMemphisMapSource to aquire map data.
 *
 */

#include "champlain-map-data-source.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_MEMPHIS
#include "champlain-debug.h"
#include "champlain-bounding-box.h"
#include "champlain-enum-types.h"
#include "champlain-tile.h"

G_DEFINE_TYPE (ChamplainMapDataSource, champlain_map_data_source, G_TYPE_INITIALLY_UNOWNED)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_MAP_DATA_SOURCE, ChamplainMapDataSourcePrivate))

enum
{
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_BOUNDING_BOX,
  PROP_STATE
};

struct _ChamplainMapDataSourcePrivate {
  ChamplainBoundingBox *bounding_box;
  /* the area that is covered by this data source */
  ChamplainState state;
  /* the state of the map data */
};

static void
champlain_map_data_source_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainMapDataSource *self = CHAMPLAIN_MAP_DATA_SOURCE (object);
  ChamplainMapDataSourcePrivate *priv = self->priv;

  switch (property_id)
    {
      case PROP_BOUNDING_BOX:
        g_value_set_boxed (value, priv->bounding_box);
        break;
      case PROP_STATE:
        g_value_set_enum (value, priv->state);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_map_data_source_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ChamplainMapDataSource *self = CHAMPLAIN_MAP_DATA_SOURCE (object);
  ChamplainMapDataSourcePrivate *priv = self->priv;

  switch (property_id)
    {
      case PROP_BOUNDING_BOX:
        champlain_bounding_box_free (priv->bounding_box);
        priv->bounding_box = g_value_dup_boxed (value);
        break;
      case PROP_STATE:
        priv->state = g_value_get_enum (value);
        g_object_notify (G_OBJECT (self), "state");
        break;
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
  ChamplainMapDataSource *self = CHAMPLAIN_MAP_DATA_SOURCE (object);
  ChamplainMapDataSourcePrivate *priv =  self->priv;

  champlain_bounding_box_free (priv->bounding_box);

  G_OBJECT_CLASS (champlain_map_data_source_parent_class)->finalize (object);
}

static MemphisMap *
champlain_map_data_source_real_get_map_data (ChamplainMapDataSource *self)
{
  g_error ("Should not be reached");
  return NULL;
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

  /**
  * ChamplainMapDataSource:bounding-box:
  *
  * The bounding box of the area that contains map data.
  *
  * Since: 0.6
  */
  g_object_class_install_property (object_class,
      PROP_BOUNDING_BOX,
      g_param_spec_boxed ("bounding-box",
        "Bounding Box",
        "The bounding box of the map data source",
        CHAMPLAIN_TYPE_BOUNDING_BOX,
        G_PARAM_READWRITE));

  /**
  * ChamplainMapDataSource:state:
  *
  * The map data source's state. Useful to know if the data source is loading
  * or not.
  *
  * Since: 0.6
  */
  g_object_class_install_property (object_class,
       PROP_STATE,
       g_param_spec_enum ("state",
           "map data source's state",
           "The state of the map data source",
           CHAMPLAIN_TYPE_STATE,
           CHAMPLAIN_STATE_NONE,
           G_PARAM_READWRITE));
}

static void
champlain_map_data_source_init (ChamplainMapDataSource *self)
{
  ChamplainMapDataSourcePrivate *priv =  GET_PRIVATE (self);

  self->priv = priv;

  priv->bounding_box = champlain_bounding_box_new ();
  priv->bounding_box->left = 0.0;
  priv->bounding_box->bottom = 0.0;
  priv->bounding_box->right = 0.0;
  priv->bounding_box->top = 0.0;

  priv->state = CHAMPLAIN_STATE_NONE;
}

/**
 * champlain_map_data_source_get_map_data:
 * @data_source: a #ChamplainMapDataSource
 *
 * Returns: the #MemphisMap of the data source or NULL.
 *
 * Since: 0.6
 */
MemphisMap *
champlain_map_data_source_get_map_data (ChamplainMapDataSource *self)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_DATA_SOURCE (self), NULL);

  return CHAMPLAIN_MAP_DATA_SOURCE_GET_CLASS (self)->get_map_data (self);
}
