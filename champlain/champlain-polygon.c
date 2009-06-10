/*
 * Copyright (C) 2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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
 * SECTION:champlain-polygon
 * @short_description: A container for #ChamplainPolygon
 *
 * A ChamplainPolygon is little more than a #ClutterContainer. It keeps the
 * polygons ordered so that they display correctly.
 *
 * Use #clutter_container_add to add polygons to the polygon and
 * #clutter_container_remove to remove them.
 */

#include "config.h"

#include "champlain-polygon.h"

#include "champlain-defines.h"
#include "champlain-private.h"

#include <clutter/clutter.h>
#include <glib.h>

static ClutterColor DEFAULT_FILL_COLOR = {0xcc, 0x00, 0x00, 0xaa};
static ClutterColor DEFAULT_STROKE_COLOR = {0xa4, 0x00, 0x00, 0xff};

G_DEFINE_TYPE (ChamplainPolygon, champlain_polygon, G_TYPE_OBJECT)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_POLYGON, ChamplainPolygonPrivate))

enum
{
  PROP_0,
  PROP_CLOSED_PATH,
  PROP_STROKE_WIDTH,
  PROP_STROKE_COLOR,
  PROP_FILL,
  PROP_FILL_COLOR,
  PROP_STROKE,
};

static void
champlain_polygon_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainPolygonPrivate *priv = GET_PRIVATE (object);

  switch (property_id)
    {
      case PROP_CLOSED_PATH:
        g_value_set_boolean (value, priv->closed_path);
        break;
      case PROP_FILL:
        g_value_set_boolean (value, priv->fill);
        break;
      case PROP_STROKE:
        g_value_set_boolean (value, priv->stroke);
        break;
      case PROP_FILL_COLOR:
        clutter_value_set_color (value, priv->fill_color);
        break;
      case PROP_STROKE_COLOR:
        clutter_value_set_color (value, priv->stroke_color);
        break;
      case PROP_STROKE_WIDTH:
        g_value_set_double (value, priv->stroke_width);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_polygon_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ChamplainPolygonPrivate *priv = GET_PRIVATE (object);

  switch (property_id)
    {
      case PROP_CLOSED_PATH:
        priv->closed_path = g_value_get_boolean (value);
        break;
      case PROP_FILL:
        champlain_polygon_set_fill (CHAMPLAIN_POLYGON (object),
            g_value_get_boolean (value));
        break;
      case PROP_STROKE:
        champlain_polygon_set_stroke (CHAMPLAIN_POLYGON (object),
            g_value_get_boolean (value));
        break;
      case PROP_FILL_COLOR:
        champlain_polygon_set_fill_color (CHAMPLAIN_POLYGON (object),
            clutter_value_get_color (value));
        break;
      case PROP_STROKE_COLOR:
        champlain_polygon_set_stroke_color (CHAMPLAIN_POLYGON (object),
            clutter_value_get_color (value));
        break;
      case PROP_STROKE_WIDTH:
        champlain_polygon_set_stroke_width (CHAMPLAIN_POLYGON (object),
            g_value_get_double (value));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_polygon_dispose (GObject *object)
{
  //ChamplainPolygonPrivate *priv = GET_PRIVATE (object);

  G_OBJECT_CLASS (champlain_polygon_parent_class)->dispose (object);
}

static void
champlain_polygon_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_polygon_parent_class)->finalize (object);
}

static void
champlain_polygon_class_init (ChamplainPolygonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainPolygonPrivate));

  object_class->get_property = champlain_polygon_get_property;
  object_class->set_property = champlain_polygon_set_property;
  object_class->dispose = champlain_polygon_dispose;
  object_class->finalize = champlain_polygon_finalize;

  /**
  * ChamplainPolygon:close-path:
  *
  * The shape is a closed path
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_CLOSED_PATH,
      g_param_spec_boolean ("closed-path",
          "Closed Path",
          "The Path is Closed",
          FALSE, CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainPolygon:fill:
  *
  * The shape should be filled
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_FILL,
      g_param_spec_boolean ("fill",
          "Fill",
          "The shape is filled",
          FALSE, CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainPolygon:stroke:
  *
  * The shape should be stroked
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_STROKE,
      g_param_spec_boolean ("stroke",
          "Stroke",
          "The shape is stroked",
          TRUE, CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainPolygon:stroke-color:
  *
  * The polygon's stroke color
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_STROKE_COLOR,
      clutter_param_spec_color ("stroke-color",
        "Stroke Color",
        "The polygon's stroke color",
        &DEFAULT_STROKE_COLOR,
        CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainPolygon:text-color:
  *
  * The polygon's fill color
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_FILL_COLOR,
      clutter_param_spec_color ("fill-color",
          "Fill Color",
          "The polygon's fill color",
          &DEFAULT_FILL_COLOR,
          CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainPolygon:stroke-width:
  *
  * The polygon's stroke width (in pixels)
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_STROKE_WIDTH,
      g_param_spec_double ("stroke-width",
          "Stroke Width",
          "The polygon's stroke width",
          0, 100.0,
          2.0,
          CHAMPLAIN_PARAM_READWRITE));
}

static void
champlain_polygon_init (ChamplainPolygon *self)
{
  self->priv = GET_PRIVATE (self);

  self->priv->points = NULL;
  self->priv->fill = FALSE;
  self->priv->stroke = TRUE;
  self->priv->stroke_width = 2.0;

  self->priv->fill_color = clutter_color_copy (&DEFAULT_FILL_COLOR);
  self->priv->stroke_color = clutter_color_copy (&DEFAULT_STROKE_COLOR);
}

/**
 * champlain_polygon_new:
 *
 * Returns a new #ChamplainPolygon ready to be to draw polygons on the map
 *
 * Since: 0.4
 */
ChamplainPolygon *
champlain_polygon_new ()
{
  return g_object_new (CHAMPLAIN_TYPE_POLYGON, NULL);
}

/**
 * champlain_polygon_append_point:
 * @polygon: The polygon
 * @lat: the latitude
 * @lon: the longitude
 *
 * Adds point at the end of the list of points in the polygon
 *
 * Returns the added point, should not be freed.
 *
 * Since: 0.4
 */
ChamplainPoint *
champlain_polygon_append_point (ChamplainPolygon *self,
    gdouble lat,
    gdouble lon)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POLYGON (self), NULL);

  ChamplainPoint *point = g_new0 (ChamplainPoint, 1);
  point->lat = lat;
  point->lon = lon;

  self->priv->points = g_list_append (self->priv->points, point);
  return point;
}

/**
 * champlain_polygon_insert_point:
 * @polygon: The polygon
 * @lat: the latitude
 * @lon: the longitude
 * @pos: where to insert the point
 *
 * Adds point at the given position in the list of points in the polygon
 *
 * Returns the added point, should not be freed.
 *
 * Since: 0.4
 */
ChamplainPoint *
champlain_polygon_insert_point (ChamplainPolygon *self,
    gdouble lat,
    gdouble lon,
    gint pos)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POLYGON (self), NULL);

  ChamplainPoint *point = g_new0 (ChamplainPoint, 1);
  point->lat = lat;
  point->lon = lon;

  self->priv->points = g_list_insert (self->priv->points, point, pos);
  return point;
}

/**
 * champlain_polygon_clear_points:
 * @polygon: The polygon
 *
 * Remove all points from the polygon
 *
 * Since: 0.4
 */
void
champlain_polygon_clear_points (ChamplainPolygon *self)
{
  g_return_if_fail (CHAMPLAIN_IS_POLYGON (self));

  GList *next = self->priv->points;
  while (next != NULL)
  {
    g_free (next->data);
    next = g_list_next (next);
  }
  g_list_free (self->priv->points);
}

/**
 * champlain_polygon_get_points:
 * @polygon: The polygon
 *
 * Returns a list of all points from the polygon, it shouldn't be freed.
 *
 * Since: 0.4
 */
GList *
champlain_polygon_get_points (ChamplainPolygon *self)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POLYGON (self), NULL);

  return self->priv->points;
}

/**
 * champlain_polygon_set_fill_color:
 * @polygon: The polygon
 * @color: The polygon's fill color or NULL to reset to the
 *         default color. The color parameter is copied.
 *
 * Set the polygon's fill color.
 *
 * Since: 0.4
 */
void
champlain_polygon_set_fill_color (ChamplainPolygon *polygon,
    const ClutterColor *color)
{
  g_return_if_fail (CHAMPLAIN_IS_POLYGON (polygon));

  ChamplainPolygonPrivate *priv = polygon->priv;

  if (priv->fill_color != NULL)
    clutter_color_free (priv->fill_color);

  if (color == NULL)
     color = &DEFAULT_FILL_COLOR;

  priv->fill_color = clutter_color_copy (color);
  g_object_notify (G_OBJECT (polygon), "fill-color");
}

/**
 * champlain_polygon_set_stoke_color:
 * @polygon: The polygon
 * @color: The polygon's stroke color or NULL to reset to the
 *         default color. The color parameter is copied.
 *
 * Set the polygon's stroke color.
 *
 * Since: 0.4
 */
void
champlain_polygon_set_stroke_color (ChamplainPolygon *polygon,
    const ClutterColor *color)
{
  g_return_if_fail (CHAMPLAIN_IS_POLYGON (polygon));

  ChamplainPolygonPrivate *priv = polygon->priv;

  if (priv->stroke_color != NULL)
    clutter_color_free (priv->stroke_color);

  if (color == NULL)
     color = &DEFAULT_STROKE_COLOR;

  priv->stroke_color = clutter_color_copy (color);
  g_object_notify (G_OBJECT (polygon), "stroke-color");
}

/**
 * champlain_polygon_get_color:
 * @polygon: The polygon
 *
 * Returns the polygon's fill color.
 *
 * Since: 0.4
 */
ClutterColor *
champlain_polygon_get_fill_color (ChamplainPolygon *polygon)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POLYGON (polygon), NULL);

  return polygon->priv->fill_color;
}

/**
 * champlain_polygon_get_stroke_color:
 * @polygon: The polygon
 *
 * Returns the polygon's stroke color.
 *
 * Since: 0.4
 */
ClutterColor *
champlain_polygon_get_stroke_color (ChamplainPolygon *polygon)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POLYGON (polygon), NULL);

  return polygon->priv->stroke_color;
}

/**
 * champlain_polygon_set_stroke:
 * @polygon: The polygon
 * @value: if the polygon is stroked
 *
 * Sets the polygon to have a stroke
 *
 * Since: 0.4
 */
void
champlain_polygon_set_stroke (ChamplainPolygon *polygon,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_POLYGON (polygon));

  polygon->priv->stroke = value;
}

/**
 * champlain_polygon_get_stroke:
 * @polygon: The polygon
 *
 * Returns if the polygon has a stroke
 *
 * Since: 0.4
 */
gboolean
champlain_polygon_get_stroke (ChamplainPolygon *polygon)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POLYGON (polygon), FALSE);

  return polygon->priv->stroke;
}

/**
 * champlain_polygon_set_fill:
 * @polygon: The polygon
 * @value: if the polygon is filled
 *
 * Sets the polygon to have be filled
 *
 * Since: 0.4
 */
void
champlain_polygon_set_fill (ChamplainPolygon *polygon,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_POLYGON (polygon));

  polygon->priv->fill = value;
}

/**
 * champlain_polygon_get_fill:
 * @polygon: The polygon
 *
 * Returns if the polygon is filled
 *
 * Since: 0.4
 */
gboolean
champlain_polygon_get_fill (ChamplainPolygon *polygon)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POLYGON (polygon), FALSE);

  return polygon->priv->fill;
}

/**
 * champlain_polygon_set_stroke_width:
 * @polygon: The polygon
 * @value: the width of the stroke (in pixels)
 *
 * Sets the width of the stroke
 *
 * Since: 0.4
 */
void
champlain_polygon_set_stroke_width (ChamplainPolygon *polygon,
    gdouble value)
{
  g_return_if_fail (CHAMPLAIN_IS_POLYGON (polygon));

  polygon->priv->stroke_width = value;
}

/**
 * champlain_polygon_get_stroke_width:
 * @polygon: The polygon
 *
 * Returns the width of the stroke
 *
 * Since: 0.4
 */
gdouble
champlain_polygon_get_stroke_width (ChamplainPolygon *polygon)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POLYGON (polygon), 0);

  return polygon->priv->stroke_width;
}
