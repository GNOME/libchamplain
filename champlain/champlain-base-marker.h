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

#ifndef CHAMPLAIN_BASE_MARKER_H
#define CHAMPLAIN_BASE_MARKER_H

#include <champlain/champlain-defines.h>

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_BASE_MARKER     (champlain_base_marker_get_type())
#define CHAMPLAIN_BASE_MARKER(obj)     (G_TYPE_CHECK_INSTANCE_CAST((obj), CHAMPLAIN_TYPE_BASE_MARKER, ChamplainBaseMarker))
#define CHAMPLAIN_BASE_MARKER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  CHAMPLAIN_TYPE_BASE_MARKER, ChamplainBaseMarkerClass))
#define CHAMPLAIN_IS_BASE_MARKER(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHAMPLAIN_TYPE_BASE_MARKER))
#define CHAMPLAIN_IS_BASE_MARKER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  CHAMPLAIN_TYPE_BASE_MARKER))
#define CHAMPLAIN_BASE_MARKER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  CHAMPLAIN_TYPE_BASE_MARKER, ChamplainBaseMarkerClass))

typedef struct _ChamplainBaseMarkerPrivate ChamplainBaseMarkerPrivate;

typedef struct _ChamplainBaseMarker ChamplainBaseMarker;
typedef struct _ChamplainBaseMarkerClass ChamplainBaseMarkerClass;


struct _ChamplainBaseMarker
{
  ClutterGroup group;

  ChamplainBaseMarkerPrivate *priv;
};

struct _ChamplainBaseMarkerClass
{
  ClutterGroupClass parent_class;

};

GType champlain_base_marker_get_type (void);

ClutterActor *champlain_base_marker_new (void);

void champlain_base_marker_set_position (ChamplainBaseMarker *marker,
    gdouble latitude, gdouble longitude);
void champlain_base_marker_set_highlighted (ChamplainBaseMarker *marker,
    gboolean value);
gboolean champlain_base_marker_get_highlighted (ChamplainBaseMarker *marker);

void champlain_base_marker_animate_in (ChamplainBaseMarker *marker);
void champlain_base_marker_animate_in_with_delay (ChamplainBaseMarker *marker,
    guint delay);
void champlain_base_marker_animate_out (ChamplainBaseMarker *marker);
void champlain_base_marker_animate_out_with_delay (ChamplainBaseMarker *marker,
    guint delay);

G_END_DECLS

#endif
