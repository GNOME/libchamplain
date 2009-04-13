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

#include <champlain/champlain-base-marker.h>

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_MARKER     (champlain_marker_get_type())
#define CHAMPLAIN_MARKER(obj)     (G_TYPE_CHECK_INSTANCE_CAST((obj), CHAMPLAIN_TYPE_MARKER, ChamplainMarker))
#define CHAMPLAIN_MARKER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  CHAMPLAIN_TYPE_MARKER, ChamplainMarkerClass))
#define CHAMPLAIN_IS_MARKER(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHAMPLAIN_TYPE_MARKER))
#define CHAMPLAIN_IS_MARKER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  CHAMPLAIN_TYPE_MARKER))
#define CHAMPLAIN_MARKER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  CHAMPLAIN_TYPE_MARKER, ChamplainMarkerClass))

typedef struct _ChamplainMarkerPrivate ChamplainMarkerPrivate;

typedef struct
{
  ChamplainBaseMarker base;

  ChamplainMarkerPrivate *priv;
} ChamplainMarker;

typedef struct
{
  ChamplainBaseMarkerClass parent_class;

  void (* draw_marker) (ChamplainMarker *marker);

} ChamplainMarkerClass;

GType champlain_marker_get_type (void);

ClutterActor *champlain_marker_new (void);

ClutterActor *champlain_marker_new_with_text (const gchar *text,
    const gchar *font, ClutterColor *text_color, ClutterColor *marker_color);

ClutterActor *champlain_marker_new_with_image (ClutterActor *actor);

ClutterActor *champlain_marker_new_from_file (const gchar *filename,
    GError **error);

ClutterActor *champlain_marker_new_full (const gchar *text,
    ClutterActor *actor);

void champlain_marker_set_text (ChamplainMarker *marker,
    const gchar *text);
void champlain_marker_set_image (ChamplainMarker *marker,
    ClutterActor *image);
void champlain_marker_set_use_markup (ChamplainMarker *marker,
    gboolean use_markup);
void champlain_marker_set_alignment (ChamplainMarker *marker,
    PangoAlignment alignment);
void champlain_marker_set_color (ChamplainMarker *marker,
    const ClutterColor *color);
void champlain_marker_set_text_color (ChamplainMarker *marker,
    const ClutterColor *color);
void champlain_marker_set_font_name (ChamplainMarker *marker,
    const gchar *font_name);
void champlain_marker_set_wrap (ChamplainMarker *marker,
    gboolean wrap);
void champlain_marker_set_wrap_mode (ChamplainMarker *marker,
    PangoWrapMode wrap_mode);
void champlain_marker_set_attributes (ChamplainMarker *marker,
    PangoAttrList *list);
void champlain_marker_set_single_line_mode (ChamplainMarker *marker,
    gboolean mode);
void champlain_marker_set_ellipsize (ChamplainMarker *marker,
    PangoEllipsizeMode mode);
void champlain_marker_set_draw_background (ChamplainMarker *marker,
    gboolean wrap);

gboolean champlain_marker_get_use_markup (ChamplainMarker *marker);
const gchar * champlain_marker_get_text (ChamplainMarker *marker);
ClutterActor * champlain_marker_get_image (ChamplainMarker *marker);
PangoAlignment champlain_marker_get_alignment (ChamplainMarker *marker);
ClutterColor * champlain_marker_get_color (ChamplainMarker *marker);
ClutterColor * champlain_marker_get_text_color (ChamplainMarker *marker);
const gchar * champlain_marker_get_font_name (ChamplainMarker *marker);
gboolean champlain_marker_get_wrap (ChamplainMarker *marker);
PangoWrapMode champlain_marker_get_wrap_mode (ChamplainMarker *marker);
PangoEllipsizeMode champlain_marker_get_ellipsize (ChamplainMarker *marker);
gboolean champlain_marker_get_single_line_mode (ChamplainMarker *marker);
gboolean champlain_marker_get_draw_background (ChamplainMarker *marker);

G_END_DECLS

#endif
