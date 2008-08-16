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

#include "config.h"

#include "champlain_defines.h"
#include "champlain_map_tile.h"
#include "champlain_map.h"
#include "champlain_widget.h"
#include "champlain-marshal.h"

#include <tidy-finger-scroll.h>
#include <tidy-scrollable.h>
#include <tidy-viewport.h>
#include <tidy-adjustment.h>
#include <math.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk-clutter-embed.h>
#include <clutter/clutter.h>

enum
{
  /* normal signals */
  TBD,
  LAST_SIGNAL
};

enum
{
  PROP_0,

  PROP_TBD
};

static guint champlain_widget_signals[LAST_SIGNAL] = { 0, };

#define CHAMPLAIN_WIDGET_GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_WIDGET, ChamplainWidgetPrivate))

typedef struct
{
  /* Units to store the origin of a click when scrolling */
  ClutterUnit x;
  ClutterUnit y;
} ChamplainPoint;

struct _ChamplainWidgetPrivate
{
  GtkWidget *clutterEmbed;
  ClutterActor *viewport;

  // Scrolling  
  ChamplainPoint position;
  ChamplainPoint hitPoint;
  
  ChamplainMap* map;
};


G_DEFINE_TYPE (ChamplainWidget, champlain_widget, GTK_TYPE_ALIGNMENT);



static void
champlain_widget_finalize (GObject * object)
{
  ChamplainWidget *widget = CHAMPLAIN_WIDGET (object);
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (widget);

  G_OBJECT_CLASS (champlain_widget_parent_class)->finalize (object);
}

static void
champlain_widget_class_init (ChamplainWidgetClass * champlainWidgetClass)
{
  g_type_class_add_private (champlainWidgetClass, sizeof (ChamplainWidgetPrivate));

  GObjectClass *objectClass = G_OBJECT_CLASS (champlainWidgetClass);
  objectClass->finalize = champlain_widget_finalize;
}

static void
champlain_widget_init (ChamplainWidget * champlainWidget)
{
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (champlainWidget);
}

static gboolean
viewport_motion_event_cb (ClutterActor * actor, ClutterMotionEvent * event, ChamplainWidget * champlainWidget)
{
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (champlainWidget);
  ClutterActor *stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->clutterEmbed));

	//FIXME: doesn't work when the viewport's size is larger than the stage (?)
  ClutterUnit x, y;
  if (clutter_actor_transform_stage_point (stage,
					   CLUTTER_UNITS_FROM_DEVICE (event->x), 
					   CLUTTER_UNITS_FROM_DEVICE (event->y), 
					   &x, 
					   &y))
    {
      ClutterUnit dx, dy;

      dx = x - priv->position.x;
      dy = y - priv->position.y;

      /*g_print ("Motion n: %d, %d\t c: %d, %d \t d: %d, %d\t h: %d, %d\n",
	       CLUTTER_UNITS_TO_INT (x), 
	       CLUTTER_UNITS_TO_INT (y),
	       CLUTTER_UNITS_TO_INT (priv->position.x),
	       CLUTTER_UNITS_TO_INT (priv->position.y), 
	       CLUTTER_UNITS_TO_INT (dx), 
	       CLUTTER_UNITS_TO_INT (dy),
	       CLUTTER_UNITS_TO_INT (priv->hitPoint.x), 
	       CLUTTER_UNITS_TO_INT (priv->hitPoint.y));*/

      priv->position.x += dx - priv->hitPoint.x;
      priv->position.y += dy - priv->hitPoint.y;

      clutter_actor_set_positionu (priv->viewport, priv->position.x, priv->position.y);
      
    } else g_print("Couldn't convert point");

  return TRUE;
}

static gboolean
viewport_button_release_event_cb (ClutterActor * actor, ClutterButtonEvent * event, ChamplainWidget * champlainWidget)
{
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (champlainWidget);
  
  ClutterActor *viewport = priv->viewport;

  if (event->button != 1)
    return FALSE;

  g_signal_handlers_disconnect_by_func (viewport, viewport_motion_event_cb, champlainWidget);
  g_signal_handlers_disconnect_by_func (viewport, viewport_button_release_event_cb, champlainWidget);

  clutter_ungrab_pointer ();

  /* Pass through events to children.
   * FIXME: this probably breaks click-count.
   */
  clutter_event_put ((ClutterEvent *) event);

  return TRUE;
}

static gboolean
after_event_cb (ChamplainWidget * champlainWidget)
{
  /* Check the pointer grab - if something else has grabbed it - for example,
   * a scroll-bar or some such, don't do our funky stuff.
   */
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (champlainWidget);
  if (clutter_get_pointer_grab () != CLUTTER_ACTOR (priv->viewport))
    {
      g_signal_handlers_disconnect_by_func (priv->viewport, viewport_motion_event_cb, champlainWidget);
      g_signal_handlers_disconnect_by_func (priv->viewport, viewport_button_release_event_cb, champlainWidget);
    }

  return FALSE;
}

static gboolean
viewport_captured_event_cb (ClutterActor * actor, ClutterEvent * event, ChamplainWidget * champlainWidget)
{

  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (champlainWidget);
  ClutterActor *stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->clutterEmbed));

  if (event->type == CLUTTER_BUTTON_PRESS)
    {
      ClutterButtonEvent *bevent = (ClutterButtonEvent *) event;
				g_print ("d: %d, %d \n", 
					bevent->x, 
					bevent->y);
      ClutterUnit x, y;
      if ((bevent->button == 1) &&
	  			(clutter_actor_transform_stage_point (stage,
						CLUTTER_UNITS_FROM_DEVICE
						(bevent->x), CLUTTER_UNITS_FROM_DEVICE (bevent->y), &x, &y)))
			{

				g_print ("Hit h: %d, %d\t c: %d, %d \t d: %d, %d \n", 
					CLUTTER_UNITS_TO_INT (x),
					CLUTTER_UNITS_TO_INT (y),
					CLUTTER_UNITS_TO_INT (priv->position.x), 
					CLUTTER_UNITS_TO_INT (priv->position.y),
					bevent->x, 
					bevent->y);
					
				priv->hitPoint.x = x - priv->position.x;
				priv->hitPoint.y = y - priv->position.y;

				clutter_grab_pointer (actor);

				/* Add a high priority idle to check the grab after the event
				 * emission is finished.
				 */
				g_idle_add_full (G_PRIORITY_HIGH_IDLE, (GSourceFunc) after_event_cb, champlainWidget, NULL);

				g_signal_connect (priv->viewport, "motion-event", G_CALLBACK (viewport_motion_event_cb), champlainWidget);
				g_signal_connect (priv->viewport,
							"button-release-event", G_CALLBACK (viewport_button_release_event_cb), champlainWidget);
			}
			else g_print("Couldn't convert point");
    }

  return FALSE;
}

static void
champlain_widget_load_map (ChamplainWidget * champlainWidget)
{
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (champlainWidget);

	priv->map = champlain_map_new(CHAMPLAIN_MAP_SOURCE_OPENSTREETMAP);
	
	champlain_map_load(priv->map, 1);
	
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport), priv->map->current_level->group);
	
}
static void
add_rects (ClutterGroup* stage) 
{
	ClutterColor white;
	clutter_color_parse("white", &white);
	ClutterColor blue;
	clutter_color_parse("blue", &blue);

	ClutterActor * rect = clutter_rectangle_new_with_color(&blue);
	clutter_container_add(CLUTTER_CONTAINER(stage), rect, NULL);
	clutter_actor_set_position (rect, 100, 100);
	clutter_actor_set_size (rect, 100, 100);
	clutter_actor_show(rect);

	rect = clutter_rectangle_new_with_color(&white);
	clutter_container_add(CLUTTER_CONTAINER(stage), rect, NULL);
	clutter_actor_set_position (rect, 100, 200);
	clutter_actor_set_size (rect, 100, 100);
	clutter_actor_show(rect);

	rect = clutter_rectangle_new_with_color(&blue);
	clutter_container_add(CLUTTER_CONTAINER(stage), rect, NULL);
	clutter_actor_set_position (rect, 200, 200);
	clutter_actor_set_size (rect, 100, 100);
	clutter_actor_show(rect);

	rect = clutter_rectangle_new_with_color(&white);
	clutter_container_add(CLUTTER_CONTAINER(stage), rect, NULL);
	clutter_actor_set_position (rect, 200, 100);
	clutter_actor_set_size (rect, 100, 100);
	clutter_actor_show(rect);

}

#define RECT_W 300
#define RECT_H 300
#define RECT_N 200
#define RECT_GAP 50
static void
viewport_x_origin_notify_cb (TidyViewport *viewport,
                             GParamSpec *args1,
                             ClutterActor *group)
{
  GList *children, *c;
  gint origin_x, width;
  
  tidy_viewport_get_origin (viewport, &origin_x, NULL, NULL);
  width = clutter_actor_get_width (
            clutter_actor_get_parent (CLUTTER_ACTOR (viewport)));
  
  children = clutter_container_get_children (CLUTTER_CONTAINER (group));
  for (c = children; c; c = c->next)
    {
      gint x;
      gdouble pos;
      ClutterActor *actor;
      
      actor = (ClutterActor *)c->data;
      
      /* Get actor position with respect to viewport origin */
      x = clutter_actor_get_x (actor) - origin_x;
      pos = (((gdouble)x / (gdouble)(width-RECT_W)) - 0.5) * 2.0;
      
      /* Apply a function that transforms the actor depending on its 
       * viewport position.
       */
      //pos = CLAMP(pos * 3.0, -0.5, 0.5);
      clutter_actor_set_position (actor, pos, pos);
    }
  g_list_free (children);
}

GtkWidget *
champlain_widget_new ()
{
  ChamplainWidget *widget = CHAMPLAIN_WIDGET (g_object_new (CHAMPLAIN_TYPE_WIDGET, NULL));
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (widget);

  priv->clutterEmbed = gtk_clutter_embed_new ();

	/* Setup stage */
  ClutterActor *stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->clutterEmbed));
  
  ClutterColor black;
  clutter_color_parse ("black", &black);
  clutter_stage_set_color (CLUTTER_STAGE (stage), &black);
  gtk_container_add (GTK_CONTAINER (widget), priv->clutterEmbed);
  
  // Setup viewport
  ClutterActor* viewport = tidy_viewport_new ();
  clutter_actor_set_clip (viewport, 0, 0, 640, 480);
  ClutterActor* group = clutter_group_new();
  add_rects (group);
	clutter_actor_show_all(group);
  clutter_container_add_actor (CLUTTER_CONTAINER (viewport), group);
  g_signal_connect (viewport, "notify::x-origin",
                    G_CALLBACK (viewport_x_origin_notify_cb), group);
  
  gdouble lower, upper;
  TidyAdjustment *hadjust, *vadjust;
  g_object_set (G_OBJECT (viewport), "sync-adjustments", FALSE, NULL);
  tidy_scrollable_get_adjustments (TIDY_SCROLLABLE (viewport), &hadjust, &vadjust);
  tidy_adjustment_get_values (hadjust, NULL, &lower, &upper, NULL, NULL, NULL);
  lower -= RECT_W - RECT_GAP;
  upper += RECT_W - RECT_GAP;
  g_object_set (hadjust, "lower", lower, "upper", upper,
                "step-increment", (gdouble)RECT_GAP, "elastic", TRUE, NULL);
  tidy_adjustment_get_values (vadjust, NULL, &lower, &upper, NULL, NULL, NULL);
  lower -= RECT_W - RECT_GAP;
  upper += RECT_W - RECT_GAP;
  g_object_set (vadjust, "lower", lower, "upper", upper,
                "step-increment", (gdouble)RECT_GAP, "elastic", TRUE, NULL);
                
  // Setup fingerscroll
  ClutterActor* finger_scroll = tidy_finger_scroll_new(TIDY_FINGER_SCROLL_MODE_PUSH);
  g_object_set (finger_scroll, "decel-rate", 1.03, NULL);
  clutter_container_add_actor (CLUTTER_CONTAINER (finger_scroll), viewport);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), finger_scroll);
  clutter_actor_set_size (finger_scroll, 640, 480); // FIXME make as wide as the stage always
  
  return GTK_WIDGET (widget);
}
