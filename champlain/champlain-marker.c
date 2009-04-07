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

/**
 * SECTION:champlain-marker
 * @short_description: A marker to identify points of interest on a map
 *
 * Markers reprensent points of interest on a map. Markers need to be placed on
 * a layer (a #ClutterGroup).  Layers have to be added to a #ChamplainView for
 * the markers to show on the map.
 *
 * A marker is nothing more than a regular #ClutterActor.  You can draw on it
 * what ever you want.  Don't forget to set the anchor position in the marker
 * using #clutter_actor_set_anchor.  Set the markers position on the map
 * using #champlain_marker_set_position.
 *
 * Champlain has a default type of markers with text. To create one,
 * use #champlain_marker_new_with_text.
 */

#include "config.h"

#include "champlain-marker.h"

#include "champlain.h"
#include "champlain-base-marker.h"
#include "champlain-defines.h"
#include "champlain-marshal.h"
#include "champlain-private.h"
#include "champlain-map.h"
#include "champlain-tile.h"
#include "champlain-zoom-level.h"

#include <clutter/clutter.h>
#include <clutter-cairo/clutter-cairo.h>
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
  PROP_IMAGE,
  PROP_TEXT,
  PROP_USE_MARKUP,
  PROP_ALIGNMENT,
  PROP_ATTRIBUTES,
  PROP_ELLIPSIZE,
  PROP_COLOR,
  PROP_TEXT_COLOR,
  PROP_FONT_NAME,
  PROP_WRAP,
  PROP_WRAP_MODE,
  PROP_SINGLE_LINE_MODE
};

//static guint champlain_marker_signals[LAST_SIGNAL] = { 0, };

struct _ChamplainMarkerPrivate
{
  gchar *text;
  ClutterActor *image;
  gboolean use_markup;
  PangoAlignment alignment;
  PangoAttrList *attributes;
  ClutterColor *color;
  ClutterColor *text_color;
  gchar *font_name;
  gboolean wrap;
  PangoWrapMode wrap_mode;
  gboolean single_line_mode;
  PangoEllipsizeMode ellipsize;

  ClutterActor *text_actor;
  ClutterActor *shadow;
  ClutterActor *background;
};

G_DEFINE_TYPE (ChamplainMarker, champlain_marker, CHAMPLAIN_TYPE_BASE_MARKER);

#define CHAMPLAIN_MARKER_GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_MARKER, ChamplainMarkerPrivate))

static void draw_marker (ChamplainMarker *marker);

static void
champlain_marker_get_property (GObject *object,
                               guint prop_id,
                               GValue *value,
                               GParamSpec *pspec)
{
    ChamplainMarker *marker = CHAMPLAIN_MARKER (object);
    ChamplainMarkerPrivate *priv = marker->priv;

    switch (prop_id)
      {
        case PROP_TEXT:
          g_value_set_string (value, priv->text);
          break;
        case PROP_IMAGE:
          g_value_set_object (value, priv->image);
          break;
        case PROP_USE_MARKUP:
          g_value_set_boolean (value, priv->use_markup);
          break;
        case PROP_ALIGNMENT:
          g_value_set_enum (value, priv->alignment);
          break;
        case PROP_COLOR:
          clutter_value_set_color (value, priv->color);
          break;
        case PROP_TEXT_COLOR:
          clutter_value_set_color (value, priv->text_color);
          break;
        case PROP_FONT_NAME:
          g_value_set_string (value, priv->font_name);
          break;
        case PROP_WRAP:
          g_value_set_boolean (value, priv->wrap);
          break;
        case PROP_WRAP_MODE:
          g_value_set_enum (value, priv->wrap_mode);
          break;
        case PROP_ELLIPSIZE:
          g_value_set_enum (value, priv->ellipsize);
          break;
        default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

static void
champlain_marker_set_property (GObject *object,
                               guint prop_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
    ChamplainMarker *marker = CHAMPLAIN_MARKER (object);
    //ChamplainMarkerPrivate *priv = marker->priv;

    switch (prop_id)
    {
      case PROP_TEXT:
        champlain_marker_set_text (marker, g_value_get_string (value));
        break;
      case PROP_IMAGE:
        champlain_marker_set_image (marker, g_value_get_object (value));
        break;
      case PROP_USE_MARKUP:
        champlain_marker_set_use_markup (marker, g_value_get_boolean (value));
        break;
      case PROP_ALIGNMENT:
        champlain_marker_set_alignment (marker, g_value_get_enum (value));
        break;
      case PROP_COLOR:
        champlain_marker_set_color (marker, clutter_value_get_color (value));
        break;
      case PROP_TEXT_COLOR:
        champlain_marker_set_text_color (marker, clutter_value_get_color (value));
        break;
      case PROP_FONT_NAME:
        champlain_marker_set_font_name (marker, g_value_get_string (value));
        break;
      case PROP_WRAP:
        champlain_marker_set_wrap (marker, g_value_get_boolean (value));
        break;
      case PROP_WRAP_MODE:
        champlain_marker_set_wrap_mode (marker, g_value_get_enum (value));
        break;
      case PROP_ELLIPSIZE:
        champlain_marker_set_ellipsize (marker, g_value_get_enum (value));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
champlain_marker_finalize (GObject *object)
{
  ChamplainMarker *marker = CHAMPLAIN_MARKER (object);
  ChamplainMarkerPrivate *priv = marker->priv;

  if (priv->text != NULL)
    g_free (priv->text);

  if (priv->image != NULL)
    g_object_unref (priv->image);

  G_OBJECT_CLASS (champlain_marker_parent_class)->finalize (object);
}

static void
champlain_marker_class_init (ChamplainMarkerClass *markerClass)
{
  g_type_class_add_private (markerClass, sizeof (ChamplainMarkerPrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (markerClass);
  object_class->finalize = champlain_marker_finalize;
  object_class->get_property = champlain_marker_get_property;
  object_class->set_property = champlain_marker_set_property;

  /**
  * ChamplainMarker:text:
  *
  * The text of the marker
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class, PROP_TEXT,
      g_param_spec_string ("text", "Text", "The text of the marker",
          "", CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainMarker:image:
  *
  * The image of the marker
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class, PROP_IMAGE,
      g_param_spec_object ("image", "Image", "The image of the marker",
          CLUTTER_TYPE_ACTOR, CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainMarker:use-markup:
  *
  * If the marker's text uses markup
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class, PROP_USE_MARKUP,
      g_param_spec_boolean ("use-markup", "Use Markup", "The text uses markup",
          FALSE, CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainMarker:alignment:
  *
  * The marker's alignment
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class, PROP_ALIGNMENT,
      g_param_spec_enum ("alignment", "Alignment", "The marker's alignment",
          PANGO_TYPE_ALIGNMENT, PANGO_ALIGN_LEFT, CHAMPLAIN_PARAM_READWRITE));

  static const ClutterColor color = {50, 50, 50, 255};
  static const ClutterColor text_color = {240, 240, 240, 255};

  /**
  * ChamplainMarker:color:
  *
  * The marker's color
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class, PROP_COLOR,
      clutter_param_spec_color ("color", "Color", "The marker's color",
          &color, CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainMarker:text-color:
  *
  * The marker's text color
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class, PROP_TEXT_COLOR,
      clutter_param_spec_color ("text-color", "Text Color", "The marker's text color",
          &text_color, CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainMarker:font-name:
  *
  * The marker's text font name
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class, PROP_FONT_NAME,
      g_param_spec_string ("font-name", "Font Name", "The marker's text font name",
          "Sans 11", CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainMarker:wrap:
  *
  * If the marker's text wrap
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class, PROP_WRAP,
      g_param_spec_boolean ("wrap", "Wrap", "The marker's text wrap",
          FALSE, CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainMarker:wrap-mode:
  *
  * If the marker's text wrap mode
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class, PROP_WRAP_MODE,
      g_param_spec_enum ("wrap-mode", "Wrap Mode", "The marker's text wrap mode",
          PANGO_TYPE_WRAP_MODE, PANGO_WRAP_WORD, CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainMarker:ellipsize:
  *
  * If the marker's ellipsize mode
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class, PROP_ELLIPSIZE,
      g_param_spec_enum ("ellipsize", "Ellipsize Mode", "The marker's text ellipsize mode",
          PANGO_TYPE_ELLIPSIZE_MODE, PANGO_ELLIPSIZE_NONE, CHAMPLAIN_PARAM_READWRITE));
}

static void
draw_marker (ChamplainMarker *marker)
{
  ChamplainMarkerPrivate *priv = marker->priv;
  ClutterColor darker_color;
  ClutterActor *actor = NULL, *text_actor, *image_actor, *bg;
  cairo_t *cr;
  guint width = 0, height = 0, point;
  guint total_width = 0, total_height = 0;
  const gint padding = 5;
  gboolean has_text = FALSE;
  gboolean has_image = FALSE;

  width = 0;
  if (priv->image != NULL)
    {
      actor = priv->image;
      clutter_actor_set_position (actor, padding, padding);
      total_width = clutter_actor_get_width (actor) + 2 * padding;
      total_height = clutter_actor_get_height (actor) + 2 * padding;
      if (clutter_actor_get_parent (actor) == NULL)
        clutter_container_add_actor (CLUTTER_CONTAINER (marker), actor);

      image_actor = actor;
      has_image = TRUE;
    }

  if (priv->text != NULL && strlen (priv->text) > 0)
    {
      ClutterLabel *label;
      if (priv->text_actor == NULL)
        actor = clutter_label_new_with_text (priv->font_name, priv->text);
      else
        actor = priv->text_actor;
      label = CLUTTER_LABEL (actor);

      clutter_label_set_use_markup (label, priv->use_markup);
      clutter_label_set_alignment (label, priv->alignment);
      clutter_label_set_line_wrap (label, priv->wrap);
      clutter_label_set_line_wrap_mode (label, priv->wrap_mode);
      clutter_label_set_ellipsize (label, priv->ellipsize);
      clutter_label_set_attributes (label, priv->attributes);

      height = clutter_actor_get_height (actor);
      if (has_image)
        clutter_actor_set_position (actor, total_width, (total_height - height) / 2.0);
      else
        clutter_actor_set_position (actor, padding, padding / 2.0);
      total_width += clutter_actor_get_width (actor) + 2 * padding;

      height += padding;
      if (height > total_height)
        total_height = height;
      clutter_label_set_color (CLUTTER_LABEL (actor), priv->text_color);
      if (clutter_actor_get_parent (actor) == NULL)
        clutter_container_add_actor (CLUTTER_CONTAINER (marker), actor);

      priv->text_actor = text_actor = actor;
      has_text = TRUE;
    }

  point = (total_height + 2 * padding) / 4.0;

  if (priv->background != NULL)
    clutter_container_remove_actor (CLUTTER_CONTAINER (marker), priv->background);

  bg = clutter_cairo_new (total_width, total_height + point);
  cr = clutter_cairo_create (CLUTTER_CAIRO (bg));

  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_move_to (cr, 0, 0);
  cairo_line_to (cr, total_width, 0);
  cairo_line_to (cr, total_width, total_height);
  cairo_line_to (cr, point, total_height);
  cairo_line_to (cr, 0, total_height + point);
  cairo_close_path (cr);

  cairo_set_line_width (cr, 1.0);
  cairo_set_source_rgba (cr,
                        priv->color->red / 255.0,
                        priv->color->green / 255.0,
                        priv->color->blue / 255.0,
                        priv->color->alpha / 255.0);
  cairo_fill_preserve (cr);
  clutter_color_darken (priv->color, &darker_color);
  cairo_set_source_rgba (cr,
                        darker_color.red / 255.0,
                        darker_color.green / 255.0,
                        darker_color.blue / 255.0,
                        darker_color.alpha / 255.0);
  cairo_stroke (cr);

  cairo_destroy (cr);
  clutter_container_add_actor (CLUTTER_CONTAINER (marker), bg);
  priv->background = bg;

  if (has_text)
    clutter_actor_raise (text_actor, bg);
  if (has_image)
  clutter_actor_raise (image_actor, bg);

  clutter_actor_set_anchor_point (CLUTTER_ACTOR (marker), 0, total_height + point);
}

static void
property_notify (GObject *gobject,
    GParamSpec *arg1,
    gpointer user_data)
{
  draw_marker (CHAMPLAIN_MARKER (gobject));
}

static void
champlain_marker_init (ChamplainMarker *marker)
{
  ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (marker) ;
  marker->priv = priv;

  priv->text = NULL;
  priv->image = NULL;
  priv->background = NULL;
  priv->use_markup = FALSE;
  priv->alignment = PANGO_ALIGN_LEFT;
  priv->attributes = NULL;
  priv->color = g_new0 (ClutterColor, 1);
  clutter_color_parse ("#333", priv->color);
  priv->text_color = g_new0 (ClutterColor, 1);
  clutter_color_parse ("#eee", priv->text_color);
  priv->font_name = g_strdup ("Sans 11");
  priv->wrap = FALSE;
  priv->wrap_mode = PANGO_WRAP_WORD;
  priv->single_line_mode = TRUE;
  priv->ellipsize = PANGO_ELLIPSIZE_NONE;

  g_signal_connect (G_OBJECT (marker), "notify", G_CALLBACK (property_notify), NULL);
}

/**
 * champlain_marker_new:
 *
 * Returns a new #ChamplainMarker ready to be used as a #ClutterActor.
 *
 * Since: 0.2
 */
ClutterActor *
champlain_marker_new (void)
{
  return CLUTTER_ACTOR (g_object_new (CHAMPLAIN_TYPE_MARKER, NULL));
}

/**
 * champlain_marker_new_with_text:
 * @text: the text of the text
 * @font: the font to use to draw the text, for example "Courrier Bold 11", can be NULL
 * @text_color: a #ClutterColor, the color of the text, can be NULL
 * @marker_color: a #ClutterColor, the color of the marker, can be NULL
 *
 * Returns a new #ChamplainMarker with a drawn marker containing the given text.
 *
 * Since: 0.2
 */
ClutterActor *
champlain_marker_new_with_text (const gchar *text,
                                 const gchar *font,
                                 ClutterColor *text_color,
                                 ClutterColor *marker_color)
{
  ChamplainMarker *marker = CHAMPLAIN_MARKER (champlain_marker_new ());

  champlain_marker_set_text (marker, text);

  if (font != NULL)
    champlain_marker_set_font_name (marker, font);

  if (text_color != NULL)
    champlain_marker_set_text_color (marker, text_color);

  if (marker_color != NULL)
    champlain_marker_set_color (marker, marker_color);

  return CLUTTER_ACTOR (marker);
}

/**
 * champlain_marker_new_with_image:
 * @filename: The filename of the image.
 * @error: Return location for an error.
 *
 * Returns a new #ChamplainMarker with a drawn marker containing the given
 * image.
 *
 * Since: 0.2
 */
ClutterActor *
champlain_marker_new_with_image (const gchar *filename, GError **error)
{
  if (filename == NULL)
    return NULL;

  ChamplainMarker *marker = CHAMPLAIN_MARKER (champlain_marker_new ());
  ClutterActor *actor = clutter_texture_new_from_file (filename, error);

  if (actor == NULL)
    {
      g_object_unref (G_OBJECT (marker));
      return NULL;
    }

  champlain_marker_set_image (marker, actor);

  return CLUTTER_ACTOR (marker);
}

/**
 * champlain_marker_new_with_image_full:
 * @filename: The name of an image file to load.
 * @width: Width of the image in pixel or -1.
 * @height: Height of the image in pixel or -1.
 * @anchor_x: X coordinate of the anchor point.
 * @anchor_y: Y coordinate of the anchor point.
 * @error: Return location for an error.
 *
 * Returns a new #ChamplainMarker with a drawn marker containing the given
 * image.
 *
 * Since: 0.4
 */
ClutterActor *
champlain_marker_new_full (const gchar *text,
    const gchar *filename,
    GError **error)
{
  if (filename == NULL)
    return NULL;

  ChamplainMarker *marker = CHAMPLAIN_MARKER (champlain_marker_new ());
  ClutterActor *actor = clutter_texture_new_from_file (filename, error);

  if (actor == NULL)
    {
      g_object_unref (G_OBJECT (marker));
      return NULL;
    }
  champlain_marker_set_image (marker, actor);
  champlain_marker_set_text (marker, text);

  return CLUTTER_ACTOR (marker);
}

/**
 * champlain_marker_set_text:
 * @marker: The marker
 * @text: The text
 *
 * Sets the marker's text.
 *
 * Since: 0.4
 */
void
champlain_marker_set_text (ChamplainMarker *marker,
    const gchar *text)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  ChamplainMarkerPrivate *priv = marker->priv;

  if (priv->text != NULL)
    g_free (priv->text);

  priv->text = g_strdup (text);
  g_object_notify (G_OBJECT (marker), "text");
}

/**
 * champlain_marker_set_image:
 * @marker: The marker
 * @image: The image as a @ClutterActor
 *
 * Sets the marker's image.
 *
 * Since: 0.4
 */
void
champlain_marker_set_image (ChamplainMarker *marker,
    ClutterActor *image)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));
  g_return_if_fail (CLUTTER_IS_ACTOR (image));

  ChamplainMarkerPrivate *priv = marker->priv;

  if (priv->image != NULL)
    g_object_unref (priv->image);

  priv->image = g_object_ref (image);
  g_object_notify (G_OBJECT (marker), "image");
}

/**
 * champlain_marker_set_use_markup:
 * @marker: The marker
 * @markup: The value
 *
 * Sets if the marker's text uses markup.
 *
 * Since: 0.4
 */
void
champlain_marker_set_use_markup (ChamplainMarker *marker,
    gboolean markup)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  ChamplainMarkerPrivate *priv = marker->priv;

  priv->use_markup = markup;
  g_object_notify (G_OBJECT (marker), "use-markup");
}

/**
 * champlain_marker_set_alignment:
 * @marker: The marker
 * @alignment: The marker's alignment
 *
 * Set the marker's text alignment.
 *
 * Since: 0.4
 */
void
champlain_marker_set_alignment (ChamplainMarker *marker,
    PangoAlignment alignment)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  ChamplainMarkerPrivate *priv = marker->priv;

  priv->alignment = alignment;
  g_object_notify (G_OBJECT (marker), "alignment");
}

/**
 * champlain_marker_set_color:
 * @marker: The marker
 * @color: The marker's background color.
 *
 * Set the marker's background color.
 *
 * Since: 0.4
 */
void
champlain_marker_set_color (ChamplainMarker *marker,
    const ClutterColor *color)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  ChamplainMarkerPrivate *priv = marker->priv;

  if (priv->color != NULL)
    clutter_color_free (priv->color);

  priv->color = clutter_color_copy (color);
  g_object_notify (G_OBJECT (marker), "color");
}

/**
 * champlain_marker_set_text_color:
 * @marker: The marker
 * @color: The marker's text color.
 *
 * Set the marker's text color.
 *
 * Since: 0.4
 */
void
champlain_marker_set_text_color (ChamplainMarker *marker,
    const ClutterColor *color)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  ChamplainMarkerPrivate *priv = marker->priv;

  if (priv->text_color != NULL)
    clutter_color_free (priv->text_color);

  priv->text_color = clutter_color_copy (color);
  g_object_notify (G_OBJECT (marker), "text-color");
}

/**
 * champlain_marker_set_text_color:
 * @marker: The marker
 * @font_name: The marker's font name.
 *
 * Set the marker's font name such as "Sans 12".
 *
 * Since: 0.4
 */
void
champlain_marker_set_font_name (ChamplainMarker *marker,
    const gchar *font_name)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  ChamplainMarkerPrivate *priv = marker->priv;

  if (priv->font_name != NULL)
    g_free (priv->font_name);

  priv->font_name = g_strdup (font_name);
  g_object_notify (G_OBJECT (marker), "font-name");
}

/**
 * champlain_marker_set_wrap:
 * @marker: The marker
 * @wrap: The marker's wrap.
 *
 * Set if the marker's text wrap.
 *
 * Since: 0.4
 */
void
champlain_marker_set_wrap (ChamplainMarker *marker,
    gboolean wrap)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  ChamplainMarkerPrivate *priv = marker->priv;

  priv->wrap = wrap;
  g_object_notify (G_OBJECT (marker), "wrap");
}

/**
 * champlain_marker_set_wrap_mode:
 * @marker: The marker
 * @wrap: The marker's wrap.
 *
 * Set the marker's text color.
 *
 * Since: 0.4
 */
void
champlain_marker_set_wrap_mode (ChamplainMarker *marker,
    PangoWrapMode wrap_mode)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  ChamplainMarkerPrivate *priv = marker->priv;

  priv->wrap_mode = wrap_mode;
  g_object_notify (G_OBJECT (marker), "wrap");
}

/**
 * champlain_marker_set_attributes:
 * @marker: The marker
 * @attributes: The marker's text attributes.
 *
 * Set the marker's text attribute.
 *
 * Since: 0.4
 */
void
champlain_marker_set_attributes (ChamplainMarker *marker,
    PangoAttrList *attributes)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  ChamplainMarkerPrivate *priv = marker->priv;

  if (attributes)
    pango_attr_list_ref (attributes);

  if (priv->attributes)
    pango_attr_list_unref (priv->attributes);

  priv->attributes = attributes;

  g_object_notify (G_OBJECT (marker), "attributes");
}

/**
 * champlain_marker_set_ellipsize:
 * @marker: The marker
 * @ellipsize: The marker's ellipsize mode.
 *
 * Set the marker's text ellipsize mode.
 *
 * Since: 0.4
 */
void
champlain_marker_set_ellipsize (ChamplainMarker *marker,
    PangoEllipsizeMode ellipsize)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  ChamplainMarkerPrivate *priv = marker->priv;

  priv->ellipsize = ellipsize;
  g_object_notify (G_OBJECT (marker), "ellipsize");
}
/**
 * champlain_marker_set_single_line_mode:
 * @marker: The marker
 * @mode: The marker's single line mode
 *
 * Set if the marker's text is on a single line.
 *
 * Since: 0.4
 */
void
champlain_marker_set_single_line_mode (ChamplainMarker *marker,
    gboolean mode)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  ChamplainMarkerPrivate *priv = marker->priv;

  priv->single_line_mode = mode;

  g_object_notify (G_OBJECT (marker), "single-line-mode");
}

/**
 * champlain_marker_get_image:
 * @marker: The marker
 *
 * Returns the marker's image.
 *
 * Since: 0.4
 */
ClutterActor *
champlain_marker_get_image (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), NULL);

  ChamplainMarkerPrivate *priv = marker->priv;

  return priv->image;
}

/**
 * champlain_marker_get_use_markup:
 * @marker: The marker
 *
 * Returns if the marker's text contains markup.
 *
 * Since: 0.4
 */
gboolean
champlain_marker_get_use_markup (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), FALSE);

  ChamplainMarkerPrivate *priv = marker->priv;

  return priv->use_markup;
}

/**
 * champlain_marker_get_text:
 * @marker: The marker
 *
 * Returns the marker's text.
 *
 * Since: 0.4
 */
const gchar *
champlain_marker_get_text (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), FALSE);

  ChamplainMarkerPrivate *priv = marker->priv;

  return priv->text;
}

/**
 * champlain_marker_get_alignment:
 * @marker: The marker
 *
 * Returns the marker's text alignment.
 *
 * Since: 0.4
 */
PangoAlignment
champlain_marker_get_alignment (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), FALSE);

  ChamplainMarkerPrivate *priv = marker->priv;

  return priv->alignment;
}

/**
 * champlain_marker_get_color:
 * @marker: The marker
 *
 * Returns the marker's color.
 *
 * Since: 0.4
 */
ClutterColor *
champlain_marker_get_color (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), FALSE);

  ChamplainMarkerPrivate *priv = marker->priv;

  return priv->color;
}

/**
 * champlain_marker_get_text_color:
 * @marker: The marker
 *
 * Returns the marker's text color.
 *
 * Since: 0.4
 */
ClutterColor *
champlain_marker_get_text_color (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), FALSE);

  ChamplainMarkerPrivate *priv = marker->priv;

  return priv->text_color;
}

/**
 * champlain_marker_get_font_name:
 * @marker: The marker
 *
 * Returns the marker's font name.
 *
 * Since: 0.4
 */
const gchar *
champlain_marker_get_font_name (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), FALSE);

  ChamplainMarkerPrivate *priv = marker->priv;

  return priv->font_name;
}

/**
 * champlain_marker_get_wrap:
 * @marker: The marker
 *
 * Returns if the marker's text wraps.
 *
 * Since: 0.4
 */
gboolean
champlain_marker_get_wrap (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), FALSE);

  ChamplainMarkerPrivate *priv = marker->priv;

  return priv->wrap;
}

/**
 * champlain_marker_get_wrap_mode:
 * @marker: The marker
 *
 * Returns the marker's text wrap mode.
 *
 * Since: 0.4
 */
PangoWrapMode
champlain_marker_get_wrap_mode (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), FALSE);

  ChamplainMarkerPrivate *priv = marker->priv;

  return priv->wrap_mode;
}

/**
 * champlain_marker_get_ellipsize:
 * @marker: The marker
 *
 * Returns the marker's text ellipsize mode;
 *
 * Since: 0.4
 */
PangoEllipsizeMode
champlain_marker_get_ellipsize (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), FALSE);

  ChamplainMarkerPrivate *priv = marker->priv;

  return priv->ellipsize;
}

/**
 * champlain_marker_get_single_line_mode:
 * @marker: The marker
 *
 * Returns the marker's text single side mode;
 *
 * Since: 0.4
 */
gboolean
champlain_marker_get_single_line_mode (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), FALSE);

  ChamplainMarkerPrivate *priv = marker->priv;

  return priv->single_line_mode;
}
