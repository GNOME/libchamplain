#include <champlain/champlain.h>
#include <clutter-cairo/clutter-cairo.h>

int
main (int argc, char *argv[])
{
  ClutterActor *view, *stage;

  g_thread_init (NULL);
  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 800, 600);

  /* Create the map view */
  actor = champlain_view_new ();
  champlain_view_set_size (CHAMPLAIN_VIEW (actor), 800, 600);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), actor);

  /* Create the marker layer */
  layer = champlain_layer_new ();
  clutter_actor_show (CLUTTER_ACTOR (layer));
  champlain_view_add_layer (CHAMPLAIN_VIEW (actor), layer);

  clutter_actor_show (stage);
  clutter_main ();

  return 0;
}
