/*
 * Copyright (C) 2011 Jiri Techet <techet@gmail.com>
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
};

/* static guint champlain_license_signals[LAST_SIGNAL] = { 0, }; */

struct _ChamplainLicensePrivate
{
  gchar *extra_text; /* Extra license text */
  ClutterActor *license_actor;
  
  ChamplainView *view;
};

G_DEFINE_TYPE (ChamplainLicense, champlain_license, CLUTTER_TYPE_GROUP);

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

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
champlain_license_dispose (GObject *object)
{
//  ChamplainLicensePrivate *priv = CHAMPLAIN_LICENSE (object)->priv;

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
champlain_license_class_init (ChamplainLicenseClass *license_class)
{
  g_type_class_add_private (license_class, sizeof (ChamplainLicensePrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (license_class);
  object_class->finalize = champlain_license_finalize;
  object_class->dispose = champlain_license_dispose;
  object_class->get_property = champlain_license_get_property;
  object_class->set_property = champlain_license_set_property;

  /**
   * ChamplainView:license-text:
   *
   * Sets additional text to be displayed in the license area.  The map's
   * license will be added below it. Your text can have multiple line, just use
   * "\n" in between.
   *
   * Since: 0.4.3
   */
  g_object_class_install_property (object_class,
      PROP_LICENSE_EXTRA,
      g_param_spec_string ("extra-text",
          "Additional license",
          "Additional license text",
          "",
          CHAMPLAIN_PARAM_READWRITE));
}


static void
redraw_license (ChamplainLicense *license)
{
  ChamplainLicensePrivate *priv = license->priv;
  gchar *text;
  gfloat width, height;
  
  if (!priv->view)
    return;

  if (priv->extra_text)
    text = g_strjoin ("\n",
          priv->extra_text,
          champlain_view_get_license_text (priv->view),
          NULL);
  else
    text = g_strdup (champlain_view_get_license_text (priv->view));

  clutter_text_set_text (CLUTTER_TEXT (priv->license_actor), text);
  clutter_actor_get_size (priv->license_actor, &width, &height);
  clutter_actor_set_size (CLUTTER_ACTOR (license), width + 2 * WIDTH_PADDING, height + 2 * HEIGHT_PADDING);
  clutter_actor_set_position (priv->license_actor, WIDTH_PADDING, HEIGHT_PADDING);

  g_free (text);
}


static void
create_license (ChamplainLicense *license)
{
  ChamplainLicensePrivate *priv = license->priv;

  if (priv->license_actor)
    {
      g_object_unref (priv->license_actor);
      clutter_container_remove_actor (CLUTTER_CONTAINER (license), priv->license_actor);
    }

  priv->license_actor = g_object_ref (clutter_text_new ());
  clutter_text_set_font_name (CLUTTER_TEXT (priv->license_actor), "sans 8");
  clutter_text_set_line_alignment (CLUTTER_TEXT (priv->license_actor), PANGO_ALIGN_RIGHT);
  clutter_actor_set_opacity (priv->license_actor, 128);
  clutter_container_add_actor (CLUTTER_CONTAINER (license), priv->license_actor);
}


static void
champlain_license_init (ChamplainLicense *license)
{
  ChamplainLicensePrivate *priv = GET_PRIVATE (license);

  license->priv = priv;
  priv->extra_text = NULL;
  priv->view = NULL;
  priv->license_actor = NULL;
  
  create_license (license);
}


ClutterActor *
champlain_license_new (void)
{
  return CLUTTER_ACTOR (g_object_new (CHAMPLAIN_TYPE_LICENSE, NULL));
}


static void
redraw_license_cb (G_GNUC_UNUSED GObject *gobject,
    G_GNUC_UNUSED GParamSpec *arg1,
    ChamplainLicense *license)
{
  redraw_license (license);
}


void 
champlain_license_connect_view (ChamplainLicense *license,
    ChamplainView *view)
{
  g_return_if_fail (CHAMPLAIN_IS_LICENSE (license));
  
  license->priv->view = view;

  g_signal_connect (view, "notify::map-source",
      G_CALLBACK (redraw_license_cb), license);
  redraw_license (license);
}

/**
 * champlain_view_set_extra_text:
 * @view: a #ChamplainView
 * @text: a license
 *
 * Show the additional license text on the map view.  The text will preceed the
 * map's licence when displayed. Use "\n" to separate the lines.
 *
 * Since: 0.4.3
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
  redraw_license (license);
}

/**
 * champlain_view_get_extra_text:
 * @view: The view
 *
 * Gets the additional license text.
 *
 * Returns: the additional license text
 *
 * Since: 0.4.3
 */
const gchar *
champlain_license_get_extra_text (ChamplainLicense *license)
{
  g_return_val_if_fail (CHAMPLAIN_IS_LICENSE (license), FALSE);

  return license->priv->extra_text;
}


