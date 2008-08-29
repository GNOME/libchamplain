/*
 * Copyright (C) 2008 Pierre-Luc Beaudoin <pierre-luc@squidy.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CHAMPLAIN_VIEW_H
#define CHAMPLAIN_VIEW_H

#include <champlain_defines.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

typedef enum
{
  CHAMPLAIN_MAP_SOURCE_DEBUG,
  CHAMPLAIN_MAP_SOURCE_OPENSTREETMAP,
  CHAMPLAIN_MAP_SOURCE_OPENARIALMAP,
  //CHAMPLAIN_MAP_SOURCE_GOOGLE_MAP,
  //CHAMPLAIN_MAP_SOURCE_GOOGLE_TERRAIN,
  //CHAMPLAIN_MAP_SOURCE_GOOGLE_SATELITE
  CHAMPLAIN_MAP_SOURCE_MAPSFORFREE_RELIEF,
  CHAMPLAIN_MAP_SOURCE_COUNT
} ChamplainMapSource;

#define CHAMPLAIN_TYPE_VIEW     (champlain_view_get_type())
#define CHAMPLAIN_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), CHAMPLAIN_TYPE_VIEW, ChamplainView))
#define CHAMPLAIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  CHAMPLAIN_TYPE_VIEW, ChamplainViewClass))
#define CHAMPLAIN_IS_VIEW(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHAMPLAIN_TYPE_VIEW))
#define CHAMPLAIN_IS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  CHAMPLAIN_TYPE_VIEW))
#define CHAMPLAIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  CHAMPLAIN_TYPE_VIEW, ChamplainViewClass))

typedef struct _ChamplainViewPrivate ChamplainViewPrivate;

/**
 * ChamplainViewMode:
 * @CHAMPLAIN_VIEW_MODE_PUSH: Non-kinetic scrolling
 * @CHAMPLAIN_VIEW_MODE_KINETIC: Kinetic scrolling
 *
 * Type of scrolling.
 */
typedef enum {
  CHAMPLAIN_VIEW_MODE_PUSH,
  CHAMPLAIN_VIEW_MODE_KINETIC
} ChamplainViewMode;

struct _ChamplainView
{
  GtkAlignment bin;

  ChamplainViewPrivate *priv;
};

struct _ChamplainViewClass
{
  GtkBinClass parent_class;

};

CHAMPLAIN_API GType champlain_view_get_type (void);

CHAMPLAIN_API GtkWidget *champlain_view_new (ChamplainViewMode mode);

CHAMPLAIN_API void champlain_view_center_on (ChamplainView *view, gdouble longitude, gdouble latitude);

CHAMPLAIN_API void champlain_view_zoom_in (ChamplainView *champlainView);

CHAMPLAIN_API void champlain_view_zoom_out (ChamplainView *champlainView);

#endif
