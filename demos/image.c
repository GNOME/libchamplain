/*
 * Copyright (C) 2009 Emmanuel Rodriguez <emmanuel.rodriguez@gmail.com>
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

#include <champlain/champlain.h>
#include <libsoup/soup.h>
#include <gdk-pixbuf/gdk-pixbuf.h>


/**
 * Transforms a pixbuf into a Clutter texture.
 */
static ClutterActor*
pixbuf_to_texture (GdkPixbuf *pixbuf, GError **error)
{
	ClutterActor *texture = NULL;
	const guchar *data;
	gboolean has_alpha, success;
	int width, height, rowstride;
	ClutterTextureFlags flags = 0;

	data = gdk_pixbuf_get_pixels (pixbuf);
	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);

	texture = clutter_texture_new ();
	success = clutter_texture_set_from_rgb_data (CLUTTER_TEXTURE (texture),
		data,
		has_alpha,
		width,
		height,
		rowstride,
		(has_alpha ? 4 : 3),
		flags,
		error);
	if (! success) {
		g_print ("Failed to create the texture\n");
		clutter_actor_destroy (CLUTTER_ACTOR (texture));
		return NULL;
	}

	g_print ("Created the texture\n");
	return texture;
}


/**
 * Called when an image has been downloaded. This callback will transform the
 * image data (binary chunk sent by the remote web server) into a valid Clutter
 * actor (a texture) and will use this as the source image for a new marker. The
 * marker will then be added to an existing layer.
 *
 * This callback expects the parameter data to be a valid ChamplainLayer.
 */
static void
image_downloaded_cb (SoupSession *session,
                     SoupMessage *message,
										 gpointer data)
{
  ChamplainLayer *layer = NULL;
	SoupURI *uri = NULL;
	char *url = NULL;
	const gchar *mime_type = NULL;
	GdkPixbufLoader *loader = NULL;
	GError *error = NULL;
	GdkPixbuf *pixbuf = NULL;
	ClutterActor *texture = NULL;
	ClutterActor *marker = NULL;

	g_print("Downloaded the image\n");
	if (data == NULL) {
		g_print ("Missing the data pointer\n");
		goto cleanup;
	}
	
	/* Deal only with finished messages */
	g_print ("message->status_code = %d\n", message->status_code);
	if (! SOUP_STATUS_IS_SUCCESSFUL(message->status_code)) {
		g_print ("SOUP message isn't finished\n");
		goto cleanup;
	}
	
	uri = soup_message_get_uri (message);
	url = soup_uri_to_string (uri, FALSE);

	/*  Deal only with successful messages */
	if (! SOUP_STATUS_IS_SUCCESSFUL (message->status_code)) {
		g_print ("Skipping download of %s since server returned error code %d\n", url, message->status_code);
		goto cleanup;
	}

	/*  Make sure that we downloaded an image */
	mime_type = soup_message_headers_get (message->response_headers, "Content-Type");


	/*  First transform the image into a pixbuf */
	loader = gdk_pixbuf_loader_new_with_mime_type (mime_type, &error);
	if (error) {
		g_print ("Can't build a PixbufLoader that will parse a %s image %s\n", mime_type, error->message);
		if (loader) {gdk_pixbuf_loader_close (loader, NULL);}
		goto cleanup;
	}
	gdk_pixbuf_loader_write (
		loader, 
		message->response_body->data,
		message->response_body->length, 
		&error);
	if (error) {
		g_print ("Can't parse the image %s: %s\n", url, error->message);
		gdk_pixbuf_loader_close (loader, NULL);
		goto cleanup;
	}

	gdk_pixbuf_loader_close (loader, &error);
	if (error) {
		g_print ("Can't close the parser for image %s: %s\n", url, error->message);
		goto cleanup;
	}
	pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
	if (pixbuf == NULL) {
		goto cleanup;
	}
	g_print ("Got a pixbuf\n");

	/* Then transform the pixbuf into a texture */
	texture = pixbuf_to_texture (pixbuf, &error);
	if (error) {
		g_print ("Can't convert the pixbuf into a texture for image %s: %s\n", url, error->message);
		goto cleanup;
	}
	g_print ("Got a texture\n");

	/* Finally create a marker with the texture */
	layer = CHAMPLAIN_LAYER (data);
	marker = champlain_marker_new_with_image (texture);
	texture = NULL;
  champlain_base_marker_set_position (CHAMPLAIN_BASE_MARKER (marker), 45.466, -73.75);
  clutter_container_add (CLUTTER_CONTAINER (layer), marker, NULL);
	clutter_actor_show_all (marker);

	/* Cleanup part, the function will always exit here even in case of error */
	cleanup:
		g_free (url);
		if (loader) {g_object_unref (G_OBJECT (loader));}
		if (texture) {clutter_actor_destroy (CLUTTER_ACTOR (texture));}
		return;
}


static void
create_marker_from_url (ChamplainLayer *layer,
                        SoupSession *session,
                        const gchar *url)
{
  SoupMessage *message;
	
	message = soup_message_new ("GET", url);
	soup_session_queue_message (session, message, image_downloaded_cb, layer);
	g_print ("Downloading %s\n",  url);
}


int
main (int argc, char *argv[])
{
  ClutterActor* actor, *stage;
  ChamplainLayer *layer;
	SoupSession *session;

  g_thread_init (NULL);
  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 800, 600);

  /* Create the map view */
  actor = champlain_view_new ();
  champlain_view_set_size (CHAMPLAIN_VIEW (actor), 800, 600);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), actor);

  /* Create the markers and marker layer */
  layer = champlain_layer_new ();
  champlain_view_add_layer (CHAMPLAIN_VIEW (actor), layer);
	session = soup_session_async_new ();
	create_marker_from_url (layer, session, "http://hexten.net/cpan-faces/potyl.jpg");

  /* Finish initialising the map view */
  g_object_set (G_OBJECT (actor), "zoom-level", 12,
      "scroll-mode", CHAMPLAIN_SCROLL_MODE_KINETIC, NULL);
  champlain_view_center_on (CHAMPLAIN_VIEW(actor), 45.466, -73.75);

  clutter_actor_show (stage);
  clutter_main ();
	
	g_object_unref (session);

  return 0;
}
