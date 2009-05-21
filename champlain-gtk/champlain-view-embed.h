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
 
#if !defined (__CHAMPLAIN_GTK_CHAMPLAIN_GTK_H_INSIDE__) && !defined (CHAMPLAIN_GTK_COMPILATION)
#error "Only <champlain/champlain.h> can be included directly."
#endif

#ifndef CHAMPLAIN_VIEW_EMBED_H
#define CHAMPLAIN_VIEW_EMBED_H

#include <gtk/gtk.h>
#include <champlain/champlain.h>

#define CHAMPLAIN_TYPE_VIEW_EMBED     (champlain_view_embed_get_type())
#define CHAMPLAIN_VIEW_EMBED(obj)     (G_TYPE_CHECK_INSTANCE_CAST((obj), CHAMPLAIN_TYPE_VIEW_EMBED, ChamplainViewEmbed))
#define CHAMPLAIN_VIEW_EMBED_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST((klass),  CHAMPLAIN_TYPE_VIEW_EMBED, ChamplainViewEmbedClass))
#define CHAMPLAIN_IS_VIEW_EMBED(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHAMPLAIN_TYPE_VIEW_EMBED))
#define CHAMPLAIN_IS_VIEW_EMBED_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  CHAMPLAIN_TYPE_VIEW_EMBED))
#define CHAMPLAIN_VIEW_EMBED_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  CHAMPLAIN_TYPE_VIEW_EMBED, ChamplainViewEmbedClass))

typedef struct _ChamplainViewEmbedPrivate ChamplainViewEmbedPrivate;


struct _ChamplainViewEmbed
{
  GtkAlignment bin;

  ChamplainViewEmbedPrivate *priv;
};

struct _ChamplainViewEmbedClass
{
  GtkAlignmentClass parent_class;

};

typedef struct _ChamplainViewEmbed ChamplainViewEmbed;

typedef struct _ChamplainViewEmbedClass ChamplainViewEmbedClass;

GType champlain_view_embed_get_type (void);

GtkWidget *champlain_view_embed_new ();

ChamplainView *champlain_view_embed_get_view (ChamplainViewEmbed* embed);
void champlain_view_embed_set_view (ChamplainViewEmbed* embed, ChamplainView *view);

#endif
