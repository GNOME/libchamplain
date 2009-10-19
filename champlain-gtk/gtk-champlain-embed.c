/*
 * Copyright (C) 2008, 2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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
 * SECTION:gtk-champlain-embed
 * @short_description: A Gtk+ Widget that embeds a #ChamplainView
 *
 * Since #ChamplainView is a #ClutterActor, you cannot embed it directly
 * into a Gtk+ application.  This widget solves this problem.  It creates
 * the #ChamplainView for you, you can get it with
 * #gtk_champlain_embed_get_view.
 */
#include "config.h"

#include <champlain/champlain.h>

#include <gtk/gtk.h>
#include <clutter/clutter.h>
#include <clutter-gtk/clutter-gtk.h>

#include "gtk-champlain-embed.h"

#if (GTK_MAJOR_VERSION == 2 && GTK_MINOR_VERSION <= 12)
#define gtk_widget_get_window(widget) ((widget)->window)
#endif

enum
{
  /* normal signals */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_VIEW
};

//static guint gtk_champlain_embed_embed_signals[LAST_SIGNAL] = { 0, };

#define GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), GTK_TYPE_CHAMPLAIN_EMBED, GtkChamplainEmbedPrivate))

struct _GtkChamplainEmbedPrivate
{
  GtkWidget *clutter_embed;
  ChamplainView *view;

  GdkCursor *cursor_hand_open;
  GdkCursor *cursor_hand_closed;

  guint width;
  guint height;
};


static void gtk_champlain_embed_get_property (GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec);
static void gtk_champlain_embed_set_property (GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec);
static void gtk_champlain_embed_finalize (GObject *object);
static void gtk_champlain_embed_dispose (GObject *object);
static void gtk_champlain_embed_class_init (GtkChamplainEmbedClass *klass);
static void gtk_champlain_embed_init (GtkChamplainEmbed *view);
static void view_size_allocated_cb (GtkWidget *widget,
    GtkAllocation *allocation, GtkChamplainEmbed *view);
static gboolean mouse_button_cb (GtkWidget *widget, GdkEventButton *event,
    GtkChamplainEmbed *view);
static void view_size_allocated_cb (GtkWidget *widget,
    GtkAllocation *allocation, GtkChamplainEmbed *view);
static void view_realize_cb (GtkWidget *widget,
    GtkChamplainEmbed *view);

G_DEFINE_TYPE (GtkChamplainEmbed, gtk_champlain_embed, GTK_TYPE_ALIGNMENT);

static void
gtk_champlain_embed_get_property (GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
  GtkChamplainEmbed *embed = GTK_CHAMPLAIN_EMBED (object);
  GtkChamplainEmbedPrivate *priv = embed->priv;

  switch(prop_id)
    {
      case PROP_VIEW:
        g_value_set_object (value, priv->view);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtk_champlain_embed_set_property (GObject *object,
    guint prop_id,
    const GValue *value,
    GParamSpec *pspec)
{
  //GtkChamplainEmbed *embed = GTK_CHAMPLAIN_EMBED (object);
  //GtkChamplainEmbedPrivate *priv = embed->priv;

  switch(prop_id)
  {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
gtk_champlain_embed_dispose (GObject *object)
{
  GtkChamplainEmbed *embed = GTK_CHAMPLAIN_EMBED (object);
  GtkChamplainEmbedPrivate *priv = embed->priv;

  if (priv->view != NULL)
    {
      g_object_unref (priv->view);
      priv->view = NULL;
    }

  if (priv->cursor_hand_open != NULL)
    {
      gdk_cursor_unref (priv->cursor_hand_open);
      priv->cursor_hand_open = NULL;
    }

  if (priv->cursor_hand_closed != NULL)
    {
      gdk_cursor_unref (priv->cursor_hand_closed);
      priv->cursor_hand_closed = NULL;
    }
}

static void
gtk_champlain_embed_finalize (GObject *object)
{
  GtkChamplainEmbed *embed = GTK_CHAMPLAIN_EMBED (object);
  GtkChamplainEmbedPrivate *priv = embed->priv;

  G_OBJECT_CLASS (gtk_champlain_embed_parent_class)->finalize (object);
}

static void
gtk_champlain_embed_class_init (GtkChamplainEmbedClass *klass)
{
  g_type_class_add_private (klass, sizeof (GtkChamplainEmbedPrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = gtk_champlain_embed_finalize;
  object_class->dispose = gtk_champlain_embed_dispose;
  object_class->get_property = gtk_champlain_embed_get_property;
  object_class->set_property = gtk_champlain_embed_set_property;

  /**
  * GtkChamplainEmbed:champlain-view:
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
         G_PARAM_READABLE));
}

static void
set_view (GtkChamplainEmbed* embed,
    ChamplainView *view)
{
  GtkChamplainEmbedPrivate *priv = embed->priv;
  ClutterActor *stage;

  stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->clutter_embed));

  if (priv->view != NULL)
    {
      g_object_unref (priv->view);
      clutter_container_remove_actor (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (priv->view));
    }

  priv->view = g_object_ref (view);
  champlain_view_set_size (priv->view, priv->width, priv->height);

  clutter_container_add_actor (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (priv->view));
}

static void
gtk_champlain_embed_init (GtkChamplainEmbed *embed)
{
  ClutterColor stage_color = { 0x34, 0x39, 0x39, 0xff };
  ClutterActor *stage;

  GtkChamplainEmbedPrivate *priv = GET_PRIVATE (embed);
  embed->priv = priv;

  priv->clutter_embed = gtk_clutter_embed_new ();

  g_signal_connect (priv->clutter_embed,
                    "size-allocate",
                    G_CALLBACK (view_size_allocated_cb),
                    embed);
  g_signal_connect (priv->clutter_embed,
                    "realize",
                    G_CALLBACK (view_realize_cb),
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
  priv->cursor_hand_open = gdk_cursor_new (GDK_HAND1);
  priv->cursor_hand_closed = gdk_cursor_new (GDK_FLEUR);

  priv->view = NULL;
  set_view (embed, CHAMPLAIN_VIEW (champlain_view_new ()));

  // Setup stage
  stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->clutter_embed));
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  gtk_container_add (GTK_CONTAINER (embed), priv->clutter_embed);

}

static void
view_realize_cb (GtkWidget *widget,
    GtkChamplainEmbed *view)
{
  ClutterColor color = {0, 0, 0, };
  GtkChamplainEmbedPrivate *priv = view->priv;

  /* Setup mouse cursor to a hand */
  gdk_window_set_cursor (gtk_widget_get_window (priv->clutter_embed), priv->cursor_hand_open);

  /* Set selection color */
  gtk_clutter_get_bg_color (GTK_WIDGET (widget), GTK_STATE_SELECTED, &color);
  champlain_marker_set_highlight_color (&color);

  gtk_clutter_get_text_color (GTK_WIDGET (widget), GTK_STATE_SELECTED, &color);
  champlain_marker_set_highlight_text_color (&color);

  /* To be added later: bg[active] (for selected markers, but focus is on another widget) */
}

static void
view_size_allocated_cb (GtkWidget *widget,
    GtkAllocation *allocation,
    GtkChamplainEmbed *view)
{
  GtkChamplainEmbedPrivate *priv = view->priv;

  if (priv->view != NULL)
    champlain_view_set_size (priv->view, allocation->width, allocation->height);

  priv->width = allocation->width;
  priv->height = allocation->height;
}

static gboolean
mouse_button_cb (GtkWidget *widget,
    GdkEventButton *event,
    GtkChamplainEmbed *view)
{
  GtkChamplainEmbedPrivate *priv = view->priv;

  if (event->type == GDK_BUTTON_PRESS)
    gdk_window_set_cursor (gtk_widget_get_window (priv->clutter_embed),
                           priv->cursor_hand_closed);
  else
    gdk_window_set_cursor (gtk_widget_get_window (priv->clutter_embed),
                           priv->cursor_hand_open);

  return FALSE;
}

/**
 * gtk_champlain_embed_new:
 *
 * Return value: a new #GtkChamplainEmbed ready to be used as a #GtkWidget.
 *
 * Since: 0.4
 */
GtkWidget *
gtk_champlain_embed_new ()
{
  return g_object_new (GTK_TYPE_CHAMPLAIN_EMBED, NULL);
}

/**
 * gtk_champlain_embed_get_view:
 * @embed: a #ChamplainView, the map view to embed
 *
 * Return value: a #ChamplainView ready to be used
 *
 * Since: 0.4
 */
ChamplainView *
gtk_champlain_embed_get_view (GtkChamplainEmbed* embed)
{
  g_return_val_if_fail (GTK_CHAMPLAIN_IS_EMBED(embed), NULL);

  GtkChamplainEmbedPrivate *priv = embed->priv;
  return priv->view;
}
