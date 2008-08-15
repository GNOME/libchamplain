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
  GtkAdjustment *horizontalAdjustment;
  GtkAdjustment *verticalAdjustment;
  ClutterActor *viewport;

  // Scrolling  
  ChamplainPoint position;
  ChamplainPoint hitPoint;
  
  ChamplainMap* map;
};


G_DEFINE_TYPE (ChamplainWidget, champlain_widget, GTK_TYPE_ALIGNMENT);

static void
adjustement_changed_cb (GtkAdjustment * adjustment, ChamplainWidgetPrivate * champlainWidget)
{
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (champlainWidget);
  /*
     if (adjustment == priv->horizontalAdjustment)
     priv->scrollOffset.x = (int)gtk_adjustment_get_value(adjustment);
     else if (adjustment == priv->verticalAdjustment)
     priv->scrollOffset.y = (int)gtk_adjustment_get_value(adjustment);
   */
  // Check if the offset is empty

}

static void
champlain_widget_set_scroll_adjustments (ChamplainWidget * champlainWidget,
					 GtkAdjustment * hadjustment, GtkAdjustment * vadjustment)
{
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (champlainWidget);

  if (priv->horizontalAdjustment)
    {
      g_signal_handlers_disconnect_by_func (G_OBJECT
					    (priv->horizontalAdjustment),
					    (gpointer) adjustement_changed_cb, champlainWidget);
      g_signal_handlers_disconnect_by_func (G_OBJECT
					    (priv->verticalAdjustment),
					    (gpointer) adjustement_changed_cb, champlainWidget);

      g_object_unref (priv->horizontalAdjustment);
      g_object_unref (priv->verticalAdjustment);
    }

  priv->horizontalAdjustment = hadjustment;
  priv->verticalAdjustment = vadjustment;

  if (hadjustment)
    {
      g_object_ref_sink (priv->horizontalAdjustment);
      g_object_ref_sink (priv->verticalAdjustment);

      gdouble val = gtk_adjustment_get_value (hadjustment);
      val = gtk_adjustment_get_value (vadjustment);
      // Connect the signals

      g_object_set (G_OBJECT (priv->horizontalAdjustment), "lower", -180.0, NULL);
      g_object_set (G_OBJECT (priv->horizontalAdjustment), "upper", 180.0, NULL);
      g_object_set (G_OBJECT (priv->horizontalAdjustment), "page-size", 20.0, NULL);
      g_object_set (G_OBJECT (priv->horizontalAdjustment), "step-increment", 5.0, NULL);
      g_object_set (G_OBJECT (priv->horizontalAdjustment), "page-increment", 15.0, NULL);

      g_object_set (G_OBJECT (priv->verticalAdjustment), "lower", -90.0, NULL);
      g_object_set (G_OBJECT (priv->verticalAdjustment), "upper", 90.0, NULL);
      g_object_set (G_OBJECT (priv->verticalAdjustment), "page-size", 20.0, NULL);
      g_object_set (G_OBJECT (priv->verticalAdjustment), "step-increment", 5.0, NULL);
      g_object_set (G_OBJECT (priv->verticalAdjustment), "page-increment", 15.0, NULL);

      //g_signal_connect(G_OBJECT(priv->horizontalAdjustment), "value-changed", (gpointer)adjustement_changed_cb, champlainWidget);
      //g_signal_connect(G_OBJECT(priv->verticalAdjustment), "value-changed", (gpointer)adjustement_changed_cb, champlainWidget);
    }
}

static void
champlain_widget_finalize (GObject * object)
{
  ChamplainWidget *widget = CHAMPLAIN_WIDGET (object);
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (widget);

  if (priv->horizontalAdjustment)
    {
      g_object_unref (priv->horizontalAdjustment);
      //g_signal_handlers_disconnect_by_func (G_OBJECT
			//		    (priv->horizontalAdjustment), (gpointer) adjustement_changed_cb, widget);
    }

  if (priv->verticalAdjustment)
    {
      g_object_unref (priv->verticalAdjustment);
      //g_signal_handlers_disconnect_by_func (G_OBJECT
			//		    (priv->verticalAdjustment), (gpointer) adjustement_changed_cb, widget);
    }

  G_OBJECT_CLASS (champlain_widget_parent_class)->finalize (object);
}

static void
champlain_widget_class_init (ChamplainWidgetClass * champlainWidgetClass)
{
  g_type_class_add_private (champlainWidgetClass, sizeof (ChamplainWidgetPrivate));

  /*
   * make us scrollable (e.g. addable to a GtkScrolledWindow)
   */
  champlainWidgetClass->set_scroll_adjustments = champlain_widget_set_scroll_adjustments;
  GTK_WIDGET_CLASS (champlainWidgetClass)->set_scroll_adjustments_signal =
    g_signal_new ("set-scroll-adjustments",
		  G_TYPE_FROM_CLASS (champlainWidgetClass),
		  (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
		  G_STRUCT_OFFSET (ChamplainWidgetClass,
				   set_scroll_adjustments), NULL, NULL,
		  champlain_marshal_VOID__OBJECT_OBJECT, G_TYPE_NONE, 2, GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);

  GObjectClass *objectClass = G_OBJECT_CLASS (champlainWidgetClass);
  objectClass->finalize = champlain_widget_finalize;
}

static void
champlain_widget_init (ChamplainWidget * champlainWidget)
{
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (champlainWidget);

  priv->horizontalAdjustment = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
  priv->verticalAdjustment = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));

  g_object_ref_sink (priv->horizontalAdjustment);
  g_object_ref_sink (priv->verticalAdjustment);


}

static gboolean
tile_is_visible(ClutterUnit viewport_w, ClutterUnit viewport_h, ChamplainPoint position, ChamplainMapTile* tile)
{
	ClutterUnit size = CLUTTER_UNITS_FROM_INT(tile->size);

      
	if( ((tile->x + 1)* size + position.x < 0 || tile->x* size + position.x > viewport_w) ||
			((tile->y + 1)* size + position.y < 0 || tile->y* size + position.y > viewport_h))
		{
			g_print ("Tile I: %d, %d\t p: %d, %d \n",
	       tile->x, tile->y,
	       CLUTTER_UNITS_TO_INT (position.x),
	       CLUTTER_UNITS_TO_INT (position.y));
			return FALSE;
		}
	g_print ("Tile V: %d, %d\t p: %d, %d \n",
	       tile->x, tile->y,
	       CLUTTER_UNITS_TO_INT (position.x),
	       CLUTTER_UNITS_TO_INT (position.y));
	return TRUE;

}

static void
champlain_widget_verify_tiles (ChamplainWidget * champlainWidget)
{
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (champlainWidget);
  ClutterActor *stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->clutterEmbed));
  ClutterUnit stage_w, stage_h;
  clutter_actor_get_sizeu(stage, &stage_w, &stage_h);
  g_print("Stage: %d, %d\n", CLUTTER_UNITS_TO_INT(stage_w), CLUTTER_UNITS_TO_INT(stage_h));
  // Check for tiles that left the viewport
  
  int i;
  int tile_count = priv->map->current_level->tiles->len;
  for (i = 0; i < tile_count; i++) 
		{
  		ChamplainMapTile* tile = g_ptr_array_index (priv->map->current_level->tiles, i);
  		if (tile_is_visible(stage_w, stage_h, priv->position, tile))
  			{
  				if(!tile->visible) 
  					{
							tile->visible = TRUE;
							clutter_container_add (CLUTTER_CONTAINER (priv->map->current_level->group), tile->actor, NULL);
						}
  			}
  			else 
  			{
  				if (tile->visible) 
  					{
							clutter_container_remove_actor (CLUTTER_CONTAINER (priv->map->current_level->group), tile->actor);
							tile->visible = FALSE;
						}
  			}
		}
		
		
  // Check for missing tiles in the viewport
  /*if ( priv->position.x > 0 )
  	{
  		gboolean exist = FALSE;
			for (i = 0; i < 20; i++) 
				{
					ClutterActor * tile = CLUTTER_ACTOR(g_ptr_array_index (priv->tiles, i));
					ClutterUnit actor_x, actor_y;
  				clutter_actor_get_positionu(tile, &actor_x, &actor_y);
					//if (actor_x < 0
				}
  
  	}*/
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

      //g_print ("Motion n: %d, %d\t c: %d, %d \t d: %d, %d\n",
	    //   CLUTTER_UNITS_TO_INT (x), CLUTTER_UNITS_TO_INT (y),
	    //   CLUTTER_UNITS_TO_INT (priv->position.x),
	    //   CLUTTER_UNITS_TO_INT (priv->position.y), CLUTTER_UNITS_TO_INT (dx), CLUTTER_UNITS_TO_INT (dy));

      priv->position.x += dx - priv->hitPoint.x;
      priv->position.y += dy - priv->hitPoint.y;

      clutter_actor_set_positionu (priv->viewport, priv->position.x, priv->position.y);
      champlain_widget_verify_tiles(champlainWidget);
    }

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
      ClutterUnit x, y;
      if ((bevent->button == 1) &&
	  			(clutter_actor_transform_stage_point (stage,
						CLUTTER_UNITS_FROM_DEVICE
						(bevent->x), CLUTTER_UNITS_FROM_DEVICE (bevent->y), &x, &y)))
			{

				//g_print ("Hit h: %d, %d\t c: %d, %d \n", CLUTTER_UNITS_TO_INT (x),
				//	 CLUTTER_UNITS_TO_INT (y),
				//	 CLUTTER_UNITS_TO_INT (priv->position.x), CLUTTER_UNITS_TO_INT (priv->position.y));
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
	champlain_widget_verify_tiles (champlainWidget);
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

	/* Setup viewport */
	priv->viewport = clutter_group_new ();
  clutter_actor_set_reactive (CLUTTER_ACTOR (priv->viewport), TRUE);
  g_signal_connect (CLUTTER_ACTOR (priv->viewport),
		    "captured-event", G_CALLBACK (viewport_captured_event_cb), widget);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), priv->viewport);
  
  champlain_widget_load_map (widget);
  
  return GTK_WIDGET (widget);
}
