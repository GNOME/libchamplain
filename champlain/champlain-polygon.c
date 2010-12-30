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
 * @short_description: A polygon to be drawn on the map
 *
 * A ChamplainPolygon is a set of point forming a shape on the map.  This API is based on Cairo's.
 */

#include "config.h"

#include "champlain-polygon.h"

#include "champlain-private.h"
#include "champlain-map-source.h"

#include <clutter/clutter.h>
#include <glib.h>
#include <math.h>

static ClutterColor DEFAULT_FILL_COLOR = { 0xcc, 0x00, 0x00, 0xaa };
static ClutterColor DEFAULT_STROKE_COLOR = { 0xa4, 0x00, 0x00, 0xff };

struct _ChamplainPolygonPrivate
{
  GList *points;
  gboolean closed_path;
  ClutterColor *stroke_color;
  gboolean fill;
  ClutterColor *fill_color;
  gboolean stroke;
  gdouble stroke_width;
  gboolean visible;
  gboolean mark_points;
};

G_DEFINE_TYPE (ChamplainPolygon, champlain_polygon, CLUTTER_TYPE_GROUP)

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
  PROP_VISIBLE,
  PROP_MARK_POINTS,
};

static void
champlain_polygon_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainPolygonPrivate *priv = CHAMPLAIN_POLYGON (object)->priv;

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

    case PROP_VISIBLE:
      g_value_set_boolean (value, priv->visible);
      break;

    case PROP_MARK_POINTS:
      g_value_set_boolean (value, priv->mark_points);
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
  ChamplainPolygonPrivate *priv = CHAMPLAIN_POLYGON (object)->priv;

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

    case PROP_VISIBLE:
      if (g_value_get_boolean (value))
        champlain_polygon_show (CHAMPLAIN_POLYGON (object));
      else
        champlain_polygon_hide (CHAMPLAIN_POLYGON (object));
      break;

    case PROP_MARK_POINTS:
      champlain_polygon_set_mark_points (CHAMPLAIN_POLYGON (object),
          g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
champlain_polygon_dispose (GObject *object)
{
  G_OBJECT_CLASS (champlain_polygon_parent_class)->dispose (object);
}


static void
champlain_polygon_finalize (GObject *object)
{
  ChamplainPolygonPrivate *priv = CHAMPLAIN_POLYGON (object)->priv;

  champlain_polygon_clear_points (CHAMPLAIN_POLYGON (object));

  clutter_color_free (priv->stroke_color);
  clutter_color_free (priv->fill_color);

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

  /**
   * ChamplainPolygon:mark-points:
   *
   * Wether the polygons points should be marked for extra visibility.
   *
   * Since: 0.4.3
   */
  g_object_class_install_property (object_class,
      PROP_MARK_POINTS,
      g_param_spec_boolean ("mark-points",
          "Mark Points",
          "The polygon's points are marked for visibility",
          FALSE,
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainPolygon:visible:
   *
   * Wether the polygon is visible
   *
   * Since: 0.4
   */
  g_object_class_install_property (object_class,
      PROP_VISIBLE,
      g_param_spec_boolean ("visible",
          "Visible",
          "The polygon's visibility",
          TRUE,
          CHAMPLAIN_PARAM_READWRITE));
}


static void
champlain_polygon_init (ChamplainPolygon *polygon)
{
  ChamplainPolygonPrivate *priv = GET_PRIVATE (polygon);

  polygon->priv = priv;

  priv->visible = TRUE;
  priv->points = NULL;
  priv->fill = FALSE;
  priv->stroke = TRUE;
  priv->stroke_width = 2.0;
  priv->mark_points = FALSE;

  priv->fill_color = clutter_color_copy (&DEFAULT_FILL_COLOR);
  priv->stroke_color = clutter_color_copy (&DEFAULT_STROKE_COLOR);
}


/**
 * champlain_polygon_new:
 *
 * Create an instance of #ChamplainPolygon.
 *
 * Returns: a new empty #ChamplainPolygon
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
 * Returns: the added point, should not be freed.
 *
 * Since: 0.4
 */
ChamplainPoint *
champlain_polygon_append_point (ChamplainPolygon *polygon,
    gdouble lat,
    gdouble lon)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POLYGON (polygon), NULL);

  ChamplainPolygonPrivate *priv = polygon->priv;

  ChamplainPoint *point = champlain_point_new (lat, lon);

  priv->points = g_list_append (priv->points, point);
  g_object_notify (G_OBJECT (polygon), "visible");
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
 * Returns: the added point, should not be freed.
 *
 * Since: 0.4
 */
ChamplainPoint *
champlain_polygon_insert_point (ChamplainPolygon *polygon,
    gdouble lat,
    gdouble lon,
    gint pos)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POLYGON (polygon), NULL);

  ChamplainPolygonPrivate *priv = polygon->priv;

  ChamplainPoint *point = champlain_point_new (lat, lon);

  priv->points = g_list_insert (priv->points, point, pos);
  g_object_notify (G_OBJECT (polygon), "visible");
  return point;
}


/**
 * champlain_polygon_remove_point:
 * @polygon: a #ChamplainPolygon
 * @point: the #ChamplainPoint to remove
 *
 * Removes the point from the polygon.
 *
 * Since: 0.4
 */
void
champlain_polygon_remove_point (ChamplainPolygon *polygon,
    ChamplainPoint *point)
{
  g_return_if_fail (CHAMPLAIN_IS_POLYGON (polygon));

  ChamplainPolygonPrivate *priv = polygon->priv;

  priv->points = g_list_remove (priv->points, point);
  g_object_notify (G_OBJECT (polygon), "visible");
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
champlain_polygon_clear_points (ChamplainPolygon *polygon)
{
  g_return_if_fail (CHAMPLAIN_IS_POLYGON (polygon));

  ChamplainPolygonPrivate *priv = polygon->priv;

  GList *next = priv->points;
  while (next != NULL)
    {
      champlain_point_free (next->data);
      next = g_list_next (next);
    }
  g_list_free (priv->points);
  priv->points = NULL;
  g_object_notify (G_OBJECT (polygon), "visible");
}


/**
 * champlain_polygon_get_points:
 * @polygon: The polygon
 *
 * Gets a list of polygon points.
 *
 * Returns: (transfer none): a list of all points from the polygon, it shouldn't be freed.
 *
 * Since: 0.4
 */
GList *
champlain_polygon_get_points (ChamplainPolygon *polygon)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POLYGON (polygon), NULL);

  return polygon->priv->points;
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
 * champlain_polygon_set_stroke_color:
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
 * champlain_polygon_get_fill_color:
 * @polygon: The polygon
 *
 * Gets the polygon's fill color.
 *
 * Returns: the polygon's fill color.
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
 * Gets the polygon's stroke color.
 *
 * Returns: the polygon's stroke color.
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
  g_object_notify (G_OBJECT (polygon), "stroke");
}


/**
 * champlain_polygon_get_stroke:
 * @polygon: The polygon
 *
 * Checks whether the polygon has a stroke.
 *
 * Returns: TRUE if the polygon has a stroke, FALSE otherwise.
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
  g_object_notify (G_OBJECT (polygon), "fill");
}


/**
 * champlain_polygon_get_fill:
 * @polygon: The polygon
 *
 * Checks whether the polygon is filled.
 *
 * Returns: TRUE if the polygon is filled, FALSE otherwise.
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
  g_object_notify (G_OBJECT (polygon), "stroke-width");
}


/**
 * champlain_polygon_get_stroke_width:
 * @polygon: The polygon
 *
 * Gets the width of the stroke.
 *
 * Returns: the width of the stroke
 *
 * Since: 0.4
 */
gdouble
champlain_polygon_get_stroke_width (ChamplainPolygon *polygon)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POLYGON (polygon), 0);

  return polygon->priv->stroke_width;
}


/**
 * champlain_polygon_set_mark_points:
 * @polygon: The polygon
 * @value: mark points when drawing the polygon.
 *
 * Sets the property determining if the points in the polygon
 * should get marked for extra visibility when drawing the polygon.
 *
 * Since: 0.4.3
 */
void
champlain_polygon_set_mark_points (ChamplainPolygon *polygon,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_POLYGON (polygon));

  polygon->priv->mark_points = value;
  g_object_notify (G_OBJECT (polygon), "mark-points");
}


/**
 * champlain_polygon_get_mark_points:
 * @polygon: The polygon
 *
 * Checks whether the polygon points are marked.
 *
 * Returns: wether points in polygon gets marked for extra visibility.
 *
 * Since: 0.4.3
 */
gboolean
champlain_polygon_get_mark_points (ChamplainPolygon *polygon)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POLYGON (polygon), FALSE);

  return polygon->priv->mark_points;
}


/**
 * champlain_polygon_show:
 * @polygon: The polygon
 *
 * Makes the polygon visible
 *
 * Since: 0.4
 */
void
champlain_polygon_show (ChamplainPolygon *polygon)
{
  g_return_if_fail (CHAMPLAIN_IS_POLYGON (polygon));

  polygon->priv->visible = TRUE;
  clutter_actor_show (CLUTTER_ACTOR (polygon));
  g_object_notify (G_OBJECT (polygon), "visible");
}


/**
 * champlain_polygon_hide:
 * @polygon: The polygon
 *
 * Hides the polygon
 *
 * Since: 0.4
 */
void
champlain_polygon_hide (ChamplainPolygon *polygon)
{
  g_return_if_fail (CHAMPLAIN_IS_POLYGON (polygon));

  polygon->priv->visible = FALSE;
  clutter_actor_hide (CLUTTER_ACTOR (polygon));
  g_object_notify (G_OBJECT (polygon), "visible");
}


void
champlain_polygon_draw_polygon (ChamplainPolygon *polygon,
    ChamplainMapSource *map_source,
    guint zoom_level,
    gfloat width,
    gfloat height,
    gfloat shift_x,
    gfloat shift_y)
{
  ChamplainPolygonPrivate *priv = polygon->priv;
  ClutterActor *cairo_texture;
  cairo_t *cr;

  if (!priv->visible || width == 0 || height == 0)
    return;

  clutter_group_remove_all (CLUTTER_GROUP (polygon));
  cairo_texture = clutter_cairo_texture_new (width, height);
  clutter_container_add_actor (CLUTTER_CONTAINER (polygon), cairo_texture);

  cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE (cairo_texture));

  /* Clear the drawing area */
  cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
  cairo_rectangle (cr, 0, 0, width, height);
  cairo_fill (cr);

  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  GList *list = g_list_first (priv->points);
  while (list != NULL)
    {
      ChamplainPoint *point = (ChamplainPoint *) list->data;
      gfloat x, y;

      x = champlain_map_source_get_x (map_source, zoom_level, point->lon);
      y = champlain_map_source_get_y (map_source, zoom_level, point->lat);

      x -= shift_x;
      y -= shift_y;

      cairo_line_to (cr, x, y);

      list = list->next;
    }

  if (priv->closed_path)
    cairo_close_path (cr);

  cairo_set_source_rgba (cr,
      priv->fill_color->red / 255.0,
      priv->fill_color->green / 255.0,
      priv->fill_color->blue / 255.0,
      priv->fill_color->alpha / 255.0);

  if (priv->fill)
    cairo_fill_preserve (cr);

  cairo_set_source_rgba (cr,
      priv->stroke_color->red / 255.0,
      priv->stroke_color->green / 255.0,
      priv->stroke_color->blue / 255.0,
      priv->stroke_color->alpha / 255.0);

  cairo_set_line_width (cr, priv->stroke_width);

  if (priv->stroke)
    cairo_stroke (cr);

  if (priv->mark_points)
    {
      /* Draw points */
      GList *list = g_list_first (priv->points);
      while (list != NULL)
        {
          ChamplainPoint *point = (ChamplainPoint *) list->data;
          gfloat x, y;

          x = champlain_map_source_get_x (map_source, zoom_level, point->lon);
          y = champlain_map_source_get_y (map_source, zoom_level, point->lat);

          x -= shift_x;
          y -= shift_y;

          cairo_arc (cr, x, y, priv->stroke_width * 1.5, 0, 2 * M_PI);
          cairo_fill (cr);

          list = list->next;
        }
    }

  cairo_destroy (cr);
}
