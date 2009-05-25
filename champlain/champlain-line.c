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
 * SECTION:champlain-line
 * @short_description: A container for #ChamplainLine
 *
 * A ChamplainLine is little more than a #ClutterContainer. It keeps the
 * lines ordered so that they display correctly.
 *
 * Use #clutter_container_add to add lines to the line and
 * #clutter_container_remove to remove them.
 */

#include "config.h"

#include "champlain-line.h"

#include "champlain-defines.h"
#include "champlain-private.h"

#include <clutter/clutter.h>
#include <glib.h>

static ClutterColor DEFAULT_FILL_COLOR = {0xcc, 0x00, 0x00, 0xaa};
static ClutterColor DEFAULT_STROKE_COLOR = {0xa4, 0x00, 0x00, 0xff};

G_DEFINE_TYPE (ChamplainLine, champlain_line, G_TYPE_OBJECT)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_LINE, ChamplainLinePrivate))

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
champlain_line_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainLinePrivate *priv = GET_PRIVATE (object);

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
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_line_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ChamplainLinePrivate *priv = GET_PRIVATE (object);

  switch (property_id)
    {
      case PROP_CLOSED_PATH:
        priv->closed_path = g_value_get_boolean (value);
        break;
      case PROP_FILL:
        priv->fill = g_value_get_boolean (value);
        break;
      case PROP_STROKE:
        priv->stroke = g_value_get_boolean (value);
        break;
      case PROP_FILL_COLOR:
        champlain_line_set_fill_color (CHAMPLAIN_LINE (object), clutter_value_get_color (value));
        break;
      case PROP_STROKE_COLOR:
        champlain_line_set_stroke_color (CHAMPLAIN_LINE (object), clutter_value_get_color (value));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_line_dispose (GObject *object)
{
  //ChamplainLinePrivate *priv = GET_PRIVATE (object);

  G_OBJECT_CLASS (champlain_line_parent_class)->dispose (object);
}

static void
champlain_line_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_line_parent_class)->finalize (object);
}

static void
champlain_line_class_init (ChamplainLineClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainLinePrivate));

  object_class->get_property = champlain_line_get_property;
  object_class->set_property = champlain_line_set_property;
  object_class->dispose = champlain_line_dispose;
  object_class->finalize = champlain_line_finalize;

  /**
  * ChamplainLine:close-path:
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
  * ChamplainLine:fill:
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
  * ChamplainLine:stroke:
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
}

static void
champlain_line_init (ChamplainLine *self)
{
  self->priv = GET_PRIVATE (self);

  self->priv->points = NULL;
  self->priv->fill = FALSE;
  self->priv->stroke = TRUE;

  self->priv->fill_color = clutter_color_copy (&DEFAULT_FILL_COLOR);
  self->priv->stroke_color = clutter_color_copy (&DEFAULT_STROKE_COLOR);
}

/**
 * champlain_line_new:
 *
 * Returns a new #ChamplainLine ready to be to draw lines on the map
 *
 * Since: 0.4
 */
ChamplainLine *
champlain_line_new ()
{
  return g_object_new (CHAMPLAIN_TYPE_LINE, NULL);
}

void
champlain_line_add_point (ChamplainLine *self,
    gdouble lat,
    gdouble lon)
{
  g_return_if_fail (CHAMPLAIN_IS_LINE (self));

  ChamplainPoint *point = g_new0 (ChamplainPoint, 1);
  point->lat = lat;
  point->lon = lon;

  self->priv->points = g_list_append (self->priv->points, point);
}

void
champlain_line_clear_points (ChamplainLine *self)
{
  g_return_if_fail (CHAMPLAIN_IS_LINE (self));

  GList *next = self->priv->points;
  while (next != NULL)
  {
    g_free (next->data);
    next = g_list_next (next);
  }
  g_list_free (self->priv->points);
}

/**
 * champlain_line_set_fill_color:
 * @line: The line
 * @color: The line's fill color or NULL to reset to the
 *         default color. The color parameter is copied.
 *
 * Set the line's fill color.
 *
 * Since: 0.4
 */
void
champlain_line_set_fill_color (ChamplainLine *line,
    const ClutterColor *color)
{
  g_return_if_fail (CHAMPLAIN_IS_LINE (line));

  ChamplainLinePrivate *priv = line->priv;

  if (priv->fill_color != NULL)
    clutter_color_free (priv->fill_color);

  if (color == NULL)
     color = &DEFAULT_FILL_COLOR;

  priv->fill_color = clutter_color_copy (color);
  g_object_notify (G_OBJECT (line), "fill-color");
}

/**
 * champlain_line_set_stoke_color:
 * @line: The line
 * @color: The line's stroke color or NULL to reset to the
 *         default color. The color parameter is copied.
 *
 * Set the line's stroke color.
 *
 * Since: 0.4
 */
void
champlain_line_set_stroke_color (ChamplainLine *line,
    const ClutterColor *color)
{
  g_return_if_fail (CHAMPLAIN_IS_LINE (line));

  ChamplainLinePrivate *priv = line->priv;

  if (priv->stroke_color != NULL)
    clutter_color_free (priv->stroke_color);

  if (color == NULL)
     color = &DEFAULT_STROKE_COLOR;

  priv->stroke_color = clutter_color_copy (color);
  g_object_notify (G_OBJECT (line), "stroke-color");
}

/**
 * champlain_line_get_color:
 * @line: The line
 *
 * Returns the line's fill color.
 *
 * Since: 0.4
 */
ClutterColor *
champlain_line_get_fill_color (ChamplainLine *line)
{
  g_return_val_if_fail (CHAMPLAIN_IS_LINE (line), NULL);

  return line->priv->fill_color;
}

/**
 * champlain_line_get_stroke_color:
 * @line: The line
 *
 * Returns the line's stroke color.
 *
 * Since: 0.4
 */
ClutterColor *
champlain_line_get_stroke_color (ChamplainLine *line)
{
  g_return_val_if_fail (CHAMPLAIN_IS_LINE (line), NULL);

  return line->priv->stroke_color;
}
