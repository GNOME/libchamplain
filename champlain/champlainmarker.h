/*
 * Copyright (C) 2008 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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

#if !defined (__CHAMPLAIN_CHAMPLAIN_H_INSIDE__) && !defined (CHAMPLAIN_COMPILATION)
#error "Only <champlain/champlain.h> can be included directly."
#endif

#ifndef CHAMPLAIN_MARKER_H
#define CHAMPLAIN_MARKER_H

#include <champlain/champlaindefines.h>
#include <glib-object.h>
#include <clutter/clutter.h>

#define CHAMPLAIN_TYPE_MARKER     (champlain_marker_get_type())
#define CHAMPLAIN_MARKER(obj)     (G_TYPE_CHECK_INSTANCE_CAST((obj), CHAMPLAIN_TYPE_MARKER, ChamplainMarker))
#define CHAMPLAIN_MARKER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  CHAMPLAIN_TYPE_MARKER, ChamplainMarkerClass))
#define CHAMPLAIN_IS_MARKER(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHAMPLAIN_TYPE_MARKER))
#define CHAMPLAIN_IS_MARKER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  CHAMPLAIN_TYPE_MARKER))
#define CHAMPLAIN_MARKER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  CHAMPLAIN_TYPE_MARKER, ChamplainMarkerClass))

typedef struct _ChamplainMarkerPrivate ChamplainMarkerPrivate;

struct _ChamplainMarker
{
  ClutterGroup group;

  ChamplainMarkerPrivate *priv;
};

struct _ChamplainMarkerClass
{
  ClutterGroupClass parent_class;

};

GType champlain_marker_get_type (void);

ClutterActor *champlain_marker_new ();

void champlain_marker_set_position (ChamplainMarker *marker, gdouble longitude, gdouble latitude);

ClutterActor *champlain_marker_new_with_label (const gchar *label,
                                 const gchar *font,
                                 ClutterColor *text_color,
                                 ClutterColor *marker_color);
#endif
