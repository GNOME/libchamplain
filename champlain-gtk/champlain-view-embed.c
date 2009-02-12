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

#include "config.h"

#include <champlain-view-embed.h>
#include <champlain/champlain.h>

#include <gtk/gtk.h>
#include <clutter/clutter.h>
#include <clutter-gtk/gtk-clutter-embed.h>

enum
{
  /* normal signals */
  LAST_SIGNAL
};

enum
{
  /* normal signals */
  PROP_0,
  PROP_VIEW
};

//static guint champlain_view_embed_embed_signals[LAST_SIGNAL] = { 0, };

#define CHAMPLAIN_VIEW_EMBED_GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_VIEW_EMBED, ChamplainViewEmbedPrivate))

struct _ChamplainViewEmbedPrivate
{
  GtkWidget *clutter_embed;
  ChamplainView *view;

  GdkCursor *cursor_hand_open;
  GdkCursor *cursor_hand_closed;
};


static void champlain_view_embed_get_property(GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec);
static void champlain_view_embed_set_property(GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec);
static void champlain_view_embed_finalize (GObject *object);
static void champlain_view_embed_class_init (ChamplainViewEmbedClass *klass);
static void champlain_view_embed_init (ChamplainViewEmbed *view);
static void view_size_allocated_cb (GtkWidget *widget,
    GtkAllocation *allocation, ChamplainViewEmbed *view);
static gboolean mouse_button_cb (GtkWidget *widget, GdkEventButton *event,
    ChamplainViewEmbed *view);
static void view_size_allocated_cb (GtkWidget *widget,
    GtkAllocation *allocation, ChamplainViewEmbed *view);

G_DEFINE_TYPE (ChamplainViewEmbed, champlain_view_embed, GTK_TYPE_ALIGNMENT);

static void
champlain_view_embed_get_property (GObject *object,
                                   guint prop_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
  ChamplainViewEmbed *embed = CHAMPLAIN_VIEW_EMBED(object);
  ChamplainViewEmbedPrivate *priv = CHAMPLAIN_VIEW_EMBED_GET_PRIVATE (embed);

  switch(prop_id)
    {
      case PROP_VIEW:
        g_value_set_object (value, priv->view);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
champlain_view_embed_set_property (GObject *object,
                                   guint prop_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
  ChamplainViewEmbed *embed = CHAMPLAIN_VIEW_EMBED(object);
  ChamplainViewEmbedPrivate *priv = CHAMPLAIN_VIEW_EMBED_GET_PRIVATE (embed);

  switch(prop_id)
  {
    case PROP_VIEW:
      {
        ClutterActor *stage;
        stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->clutter_embed));

        if (priv->view != NULL)
          {
            g_object_unref (priv->view);
            clutter_container_remove_actor (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (priv->view));
          }

        priv->view = g_value_dup_object (value);
        clutter_container_add_actor (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (priv->view));
        break;
      }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}

static void
champlain_view_embed_finalize (GObject *object)
{
  ChamplainViewEmbed *embed = CHAMPLAIN_VIEW_EMBED (object);
  ChamplainViewEmbedPrivate *priv = CHAMPLAIN_VIEW_EMBED_GET_PRIVATE (embed);

  g_object_unref (priv->view);
  G_OBJECT_CLASS (champlain_view_embed_parent_class)->finalize (object);
}

static void
champlain_view_embed_class_init (ChamplainViewEmbedClass *klass)
{
  g_type_class_add_private (klass, sizeof (ChamplainViewEmbedPrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = champlain_view_embed_finalize;
  object_class->get_property = champlain_view_embed_get_property;
  object_class->set_property = champlain_view_embed_set_property;

  /**
  * ChamplainViewEmbed:champlain-view:
  *
  * The #ChamplainView to embed in the Gtk+ widget.
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_VIEW,
      g_param_spec_object ("champlain-view",
         "Champlain view",
         "The ChamplainView to embed into the Gtk+ widget",
         CHAMPLAIN_TYPE_VIEW,
         CHAMPLAIN_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void
champlain_view_embed_init (ChamplainViewEmbed *embed)
{
  ClutterColor stage_color = { 0x34, 0x39, 0x39, 0xff };
  ClutterActor *stage;

  ChamplainViewEmbedPrivate *priv = CHAMPLAIN_VIEW_EMBED_GET_PRIVATE (embed);

  priv->view = NULL;
  priv->clutter_embed = gtk_clutter_embed_new ();

  g_signal_connect (priv->clutter_embed,
                    "size-allocate",
                    G_CALLBACK (view_size_allocated_cb),
                    embed);
  g_signal_connect (priv->clutter_embed,
                    "button-press-event",
                    G_CALLBACK (mouse_button_cb),
                    embed);
  g_signal_connect (priv->clutter_embed,
                    "button-release-event",
                    G_CALLBACK (mouse_button_cb),
                    embed);
  // Setup cursors
  priv->cursor_hand_open = gdk_cursor_new(GDK_HAND1);
  priv->cursor_hand_closed = gdk_cursor_new(GDK_FLEUR);

  // Setup stage
  stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->clutter_embed));
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  gtk_container_add (GTK_CONTAINER (embed), priv->clutter_embed);
}

static void
view_size_allocated_cb (GtkWidget *widget,
                        GtkAllocation *allocation,
                        ChamplainViewEmbed *view)
{
  ChamplainViewEmbedPrivate *priv = CHAMPLAIN_VIEW_EMBED_GET_PRIVATE (view);

  champlain_view_set_size(priv->view, allocation->width, allocation->height);

  // Setup mouse cursor to a hand
  gdk_window_set_cursor( priv->clutter_embed->window, priv->cursor_hand_open);
}

static gboolean
mouse_button_cb (GtkWidget *widget,
                 GdkEventButton *event,
                 ChamplainViewEmbed *view)
{
  ChamplainViewEmbedPrivate *priv = CHAMPLAIN_VIEW_EMBED_GET_PRIVATE (view);

  if (event->type == GDK_BUTTON_PRESS)
    gdk_window_set_cursor( priv->clutter_embed->window, priv->cursor_hand_closed);
  else
    gdk_window_set_cursor( priv->clutter_embed->window, priv->cursor_hand_open);

  return FALSE;
}

/**
 * champlain_view_embed_new:
 * @mode: a #ChamplainView, the map view to embed
 * Returns a new #ChamplainViewEmbed ready to be used as a #GtkWidget.
 *
 * Since: 0.2.1
 */
GtkWidget *
champlain_view_embed_new (ChamplainView *view)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), NULL);

  return g_object_new (CHAMPLAIN_TYPE_VIEW_EMBED, "champlain-view", view, NULL);
}

ChamplainView *
champlain_view_embed_get_view (ChamplainViewEmbed* embed)
{
  g_return_val_if_fail(CHAMPLAIN_IS_VIEW_EMBED(embed), NULL);

  ChamplainViewEmbedPrivate *priv = CHAMPLAIN_VIEW_EMBED_GET_PRIVATE (embed);
  return priv->view;
}
