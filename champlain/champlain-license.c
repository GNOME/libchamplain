/*
 * Copyright (C) 2011-2012 Jiri Techet <techet@gmail.com>
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
 * SECTION:champlain-license
 * @short_description: An actor that displays license text.
 *
 * An actor that displays license text.
 */

#include "config.h"

#include "champlain-license.h"
#include "champlain-defines.h"
#include "champlain-marshal.h"
#include "champlain-private.h"
#include "champlain-enum-types.h"
#include "champlain-view.h"

#include <clutter/clutter.h>
#include <glib.h>
#include <glib-object.h>
#include <cairo.h>
#include <math.h>
#include <string.h>


enum
{
  /* normal signals */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_LICENSE_EXTRA,
  PROP_ALIGNMENT,
};

/* static guint champlain_license_signals[LAST_SIGNAL] = { 0, }; */

struct _ChamplainLicensePrivate
{
  gchar *extra_text; /* Extra license text */
  ClutterActor *license_actor;
  ClutterGroup *content_group;
  PangoAlignment alignment;

  ChamplainView *view;
};

G_DEFINE_TYPE (ChamplainLicense, champlain_license, CLUTTER_TYPE_ACTOR);

#define GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CHAMPLAIN_TYPE_LICENSE, ChamplainLicensePrivate))

#define WIDTH_PADDING 10
#define HEIGHT_PADDING 7


static void
champlain_license_get_property (GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainLicensePrivate *priv = CHAMPLAIN_LICENSE (object)->priv;

  switch (prop_id)
    {
    case PROP_LICENSE_EXTRA:
      g_value_set_string (value, priv->extra_text);
      break;

    case PROP_ALIGNMENT:
      g_value_set_enum (value, priv->alignment);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
champlain_license_set_property (GObject *object,
    guint prop_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ChamplainLicense *license = CHAMPLAIN_LICENSE (object);

  switch (prop_id)
    {
    case PROP_LICENSE_EXTRA:
      champlain_license_set_extra_text (license, g_value_get_string (value));
      break;

    case PROP_ALIGNMENT:
      champlain_license_set_alignment (license, g_value_get_enum (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
paint (ClutterActor *self)
{
  ChamplainLicensePrivate *priv = GET_PRIVATE (self);

  clutter_actor_paint (CLUTTER_ACTOR (priv->content_group));
}


static void
pick (ClutterActor *self,
    const ClutterColor *color)
{
  ChamplainLicensePrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_license_parent_class)->pick (self, color);

  clutter_actor_paint (CLUTTER_ACTOR (priv->content_group));
}


static void
get_preferred_width (ClutterActor *self,
    gfloat for_height,
    gfloat *min_width_p,
    gfloat *natural_width_p)
{
  ChamplainLicensePrivate *priv = GET_PRIVATE (self);

  clutter_actor_get_preferred_width (CLUTTER_ACTOR (priv->content_group),
      for_height,
      min_width_p,
      natural_width_p);
}


static void
get_preferred_height (ClutterActor *self,
    gfloat for_width,
    gfloat *min_height_p,
    gfloat *natural_height_p)
{
  ChamplainLicensePrivate *priv = GET_PRIVATE (self);

  clutter_actor_get_preferred_height (CLUTTER_ACTOR (priv->content_group),
      for_width,
      min_height_p,
      natural_height_p);
}


static void
allocate (ClutterActor *self,
    const ClutterActorBox *box,
    ClutterAllocationFlags flags)
{
  ClutterActorBox child_box;

  ChamplainLicensePrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_license_parent_class)->allocate (self, box, flags);

  child_box.x1 = 0;
  child_box.x2 = box->x2 - box->x1;
  child_box.y1 = 0;
  child_box.y2 = box->y2 - box->y1;

  clutter_actor_allocate (CLUTTER_ACTOR (priv->content_group), &child_box, flags);
}


static void
map (ClutterActor *self)
{
  ChamplainLicensePrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_license_parent_class)->map (self);

  clutter_actor_map (CLUTTER_ACTOR (priv->content_group));
}


static void
unmap (ClutterActor *self)
{
  ChamplainLicensePrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_license_parent_class)->unmap (self);

  clutter_actor_unmap (CLUTTER_ACTOR (priv->content_group));
}


static void
redraw_license (ChamplainLicense *license)
{
  ChamplainLicensePrivate *priv = license->priv;
  gchar *text;
  gfloat width, height;
  ChamplainMapSource *map_source;

  if (!priv->view)
    return;

  map_source = champlain_view_get_map_source (priv->view);

  if (!map_source)
    return;

  if (priv->extra_text)
    text = g_strjoin ("\n",
          priv->extra_text,
          champlain_map_source_get_license (map_source),
          NULL);
  else
    text = g_strdup (champlain_map_source_get_license (map_source));

  clutter_text_set_text (CLUTTER_TEXT (priv->license_actor), text);
  clutter_actor_get_size (priv->license_actor, &width, &height);
  clutter_actor_set_size (CLUTTER_ACTOR (license), width + 2 * WIDTH_PADDING, height + 2 * HEIGHT_PADDING);
  clutter_actor_set_position (priv->license_actor, WIDTH_PADDING, HEIGHT_PADDING);

  g_free (text);
}


static void
redraw_license_cb (G_GNUC_UNUSED GObject *gobject,
    G_GNUC_UNUSED GParamSpec *arg1,
    ChamplainLicense *license)
{
  redraw_license (license);
}


static void
champlain_license_dispose (GObject *object)
{
  ChamplainLicensePrivate *priv = CHAMPLAIN_LICENSE (object)->priv;

  if (priv->content_group)
    {
      clutter_actor_unparent (CLUTTER_ACTOR (priv->content_group));
      priv->content_group = NULL;
    }

  priv->license_actor = NULL;

  if (priv->view)
    {
      champlain_license_disconnect_view (CHAMPLAIN_LICENSE (object));
      priv->view = NULL;
    }

  G_OBJECT_CLASS (champlain_license_parent_class)->dispose (object);
}


static void
champlain_license_finalize (GObject *object)
{
  ChamplainLicensePrivate *priv = CHAMPLAIN_LICENSE (object)->priv;

  g_free (priv->extra_text);

  G_OBJECT_CLASS (champlain_license_parent_class)->finalize (object);
}


static void
champlain_license_class_init (ChamplainLicenseClass *klass)
{
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainLicensePrivate));

  object_class->finalize = champlain_license_finalize;
  object_class->dispose = champlain_license_dispose;
  object_class->get_property = champlain_license_get_property;
  object_class->set_property = champlain_license_set_property;

  actor_class->get_preferred_width = get_preferred_width;
  actor_class->get_preferred_height = get_preferred_height;
  actor_class->allocate = allocate;
  actor_class->paint = paint;
  actor_class->pick = pick;
  actor_class->map = map;
  actor_class->unmap = unmap;

  /**
   * ChamplainLicense:extra-text:
   *
   * Sets additional text to be displayed in the license area.  The map's
   * license will be added below it. Your text can have multiple lines, just use
   * "\n" in between.
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_LICENSE_EXTRA,
      g_param_spec_string ("extra-text",
          "Additional license",
          "Additional license text",
          "",
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainLicense:alignment:
   *
   * The license's alignment
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class, 
      PROP_ALIGNMENT,
      g_param_spec_enum ("alignment", 
          "Alignment", 
          "The license's alignment",
          PANGO_TYPE_ALIGNMENT, 
          PANGO_ALIGN_LEFT, 
          CHAMPLAIN_PARAM_READWRITE));
}


static void
champlain_license_init (ChamplainLicense *license)
{
  ChamplainLicensePrivate *priv = GET_PRIVATE (license);

  license->priv = priv;
  priv->extra_text = NULL;
  priv->view = NULL;
  priv->alignment = PANGO_ALIGN_RIGHT;
  priv->content_group = CLUTTER_GROUP (clutter_group_new ());
  clutter_actor_set_parent (CLUTTER_ACTOR (priv->content_group), CLUTTER_ACTOR (license));

  priv->license_actor = clutter_text_new ();
  clutter_text_set_font_name (CLUTTER_TEXT (priv->license_actor), "sans 8");
  clutter_text_set_line_alignment (CLUTTER_TEXT (priv->license_actor), priv->alignment);
  clutter_actor_set_opacity (priv->license_actor, 128);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->content_group), priv->license_actor);
  
  clutter_actor_queue_relayout (CLUTTER_ACTOR (license));
}


/**
 * champlain_license_new:
 *
 * Creates an instance of #ChamplainLicense.
 *
 * Returns: a new #ChamplainLicense.
 *
 * Since: 0.10
 */
ClutterActor *
champlain_license_new (void)
{
  return CLUTTER_ACTOR (g_object_new (CHAMPLAIN_TYPE_LICENSE, NULL));
}


/**
 * champlain_license_connect_view:
 * @license: The license
 * @view: a #ChamplainView
 *
 * This method connects to the necessary signals of #ChamplainView to make the
 * license change automatically when the map source changes.
 *
 * Since: 0.10
 */
void
champlain_license_connect_view (ChamplainLicense *license,
    ChamplainView *view)
{
  g_return_if_fail (CHAMPLAIN_IS_LICENSE (license));

  license->priv->view = g_object_ref (view);

  g_signal_connect (view, "notify::map-source",
      G_CALLBACK (redraw_license_cb), license);
  redraw_license (license);
}


/**
 * champlain_license_disconnect_view:
 * @license: The license
 *
 * This method disconnects from the signals previously connected by champlain_license_connect_view().
 *
 * Since: 0.10
 */
void
champlain_license_disconnect_view (ChamplainLicense *license)
{
  g_return_if_fail (CHAMPLAIN_IS_LICENSE (license));

  g_signal_handlers_disconnect_by_func (license->priv->view,
      redraw_license_cb,
      license);
  g_object_unref (license->priv->view);
  license->priv->view = NULL;
}


/**
 * champlain_license_set_extra_text:
 * @license: a #ChamplainLicense
 * @text: a license
 *
 * Show the additional license text on the map view.  The text will preceed the
 * map's licence when displayed. Use "\n" to separate the lines.
 *
 * Since: 0.10
 */
void
champlain_license_set_extra_text (ChamplainLicense *license,
    const gchar *text)
{
  g_return_if_fail (CHAMPLAIN_IS_LICENSE (license));

  ChamplainLicensePrivate *priv = license->priv;

  if (priv->extra_text)
    g_free (priv->extra_text);

  priv->extra_text = g_strdup (text);
  g_object_notify (G_OBJECT (license), "extra-text");
  redraw_license (license);
}


/**
 * champlain_license_get_extra_text:
 * @license: a #ChamplainLicense
 *
 * Gets the additional license text.
 *
 * Returns: the additional license text
 *
 * Since: 0.10
 */
const gchar *
champlain_license_get_extra_text (ChamplainLicense *license)
{
  g_return_val_if_fail (CHAMPLAIN_IS_LICENSE (license), FALSE);

  return license->priv->extra_text;
}


/**
 * champlain_license_set_alignment:
 * @license: The license
 * @alignment: The license's alignment
 *
 * Set the license's text alignment.
 *
 * Since: 0.10
 */
void
champlain_license_set_alignment (ChamplainLicense *license,
    PangoAlignment alignment)
{
  g_return_if_fail (CHAMPLAIN_IS_LICENSE (license));

  license->priv->alignment = alignment;
  clutter_text_set_line_alignment (CLUTTER_TEXT (license->priv->license_actor), alignment);
  g_object_notify (G_OBJECT (license), "alignment");
}


/**
 * champlain_license_get_alignment:
 * @license: The license
 *
 * Get the license's text alignment.
 *
 * Returns: the license's text alignment.
 *
 * Since: 0.10
 */
PangoAlignment
champlain_license_get_alignment (ChamplainLicense *license)
{
  g_return_val_if_fail (CHAMPLAIN_IS_LICENSE (license), FALSE);

  return license->priv->alignment;
}
