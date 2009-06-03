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
 * @short_description: A container for #ChamplainMarker
 *
 * A ChamplainLine is little more than a #ClutterContainer. It keeps the
 * markers ordered so that they display correctly.
 *
 * Use #clutter_container_add to add markers to the line and
 * #clutter_container_remove to remove them.
 */

#include "config.h"

#include "champlain-line.h"

#include "champlain-defines.h"

#include <clutter/clutter.h>
#include <glib.h>

G_DEFINE_TYPE (ChamplainLine, champlain_line, G_TYPE_OBJECT)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_LINE, ChamplainLinePrivate))

enum
{
  PROP_0
};

struct _ChamplainLinePrivate {
  GList *points;
};

static void
champlain_line_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  //ChamplainLine *self = CHAMPLAIN_LINE (object);
  switch (property_id)
    {
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
  //ChamplainLine *self = CHAMPLAIN_LINE (object);
  switch (property_id)
    {
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
}

static void
champlain_line_init (ChamplainLine *self)
{
  self->priv = GET_PRIVATE (self);

  self->priv->points = NULL;
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

