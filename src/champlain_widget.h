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

#ifndef CHAMPLAIN_WIDGET_H
#define CHAMPLAIN_WIDGET_H

#include <champlain_defines.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS
#define CHAMPLAIN_TYPE_WIDGET     (champlain_widget_get_type())
#define CHAMPLAIN_WIDGET(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), CHAMPLAIN_TYPE_WIDGET, ChamplainWidget))
#define CHAMPLAIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  CHAMPLAIN_TYPE_WIDGET, ChamplainWidgetClass))
#define CHAMPLAIN_IS_WIDGET(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHAMPLAIN_TYPE_WIDGET))
#define CHAMPLAIN_IS_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  CHAMPLAIN_TYPE_WIDGET))
#define CHAMPLAIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  CHAMPLAIN_TYPE_WIDGET, ChamplainWidgetClass))
typedef struct _ChamplainWidgetPrivate ChamplainWidgetPrivate;

struct _ChamplainWidget
{
  GtkAlignment bin;

  ChamplainWidgetPrivate *priv;
};

struct _ChamplainWidgetClass
{
  GtkBinClass parent_class;


  ChamplainWidget *(*create_widget) (ChamplainWidget * widget);

  void (*set_scroll_adjustments) (ChamplainWidget * widget, GtkAdjustment * hadjustment, GtkAdjustment * vadjustment);

};

CHAMPLAIN_API GType champlain_widget_get_type (void);

CHAMPLAIN_API GtkWidget *champlain_widget_new (void);

#endif
