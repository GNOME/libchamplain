/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Authored By Matthew Allum  <mallum@openedhand.com>
 *
 * Copyright (C) 2006 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __CHAMPLAIN_GROUP_H__
#define __CHAMPLAIN_GROUP_H__

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_GROUP champlain_group_get_type ()

#define CHAMPLAIN_GROUP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_GROUP, ChamplainGroup))

#define CHAMPLAIN_GROUP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_GROUP, ChamplainGroupClass))

#define CHAMPLAIN_IS_GROUP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_GROUP))

#define CHAMPLAIN_IS_GROUP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_GROUP))

#define CHAMPLAIN_GROUP_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_GROUP, ChamplainGroupClass))

typedef struct _ChamplainGroup ChamplainGroup;
typedef struct _ChamplainGroupClass ChamplainGroupClass;
typedef struct _ChamplainGroupPrivate ChamplainGroupPrivate;

/**
 * ChamplainGroup:
 *
 * The #ChamplainGroup structure contains only private data
 * and should be accessed using the provided API
 *
 * Since: 0.1
 */
struct _ChamplainGroup
{
  /*< private >*/
  ClutterActor parent_instance;

  ChamplainGroupPrivate *priv;
};

/**
 * ChamplainGroupClass:
 *
 * The #ChamplainGroupClass structure contains only private data
 *
 * Since: 0.1
 */
struct _ChamplainGroupClass
{
  /*< private >*/
  ClutterActorClass parent_class;

  /* padding for future expansion */
  void (*_clutter_reserved1)(void);
  void (*_clutter_reserved2)(void);
  void (*_clutter_reserved3)(void);
  void (*_clutter_reserved4)(void);
  void (*_clutter_reserved5)(void);
  void (*_clutter_reserved6)(void);
};

GType champlain_group_get_type (void) G_GNUC_CONST;
ClutterActor *champlain_group_new (void);
ClutterActor *champlain_group_get_nth_child (ChamplainGroup *self,
    gint index_);
gint champlain_group_get_n_children (ChamplainGroup *self);
void champlain_group_remove_all (ChamplainGroup *group);

/* for Mr. Mallum */
#define champlain_group_add(group, actor)                  G_STMT_START {  \
    ClutterActor *_actor = (ClutterActor *) (actor);                      \
    if (CHAMPLAIN_IS_GROUP ((group)) && CLUTTER_IS_ACTOR ((_actor)))        \
      {                                                                   \
        ClutterContainer *_container = (ClutterContainer *) (group);      \
        clutter_container_add_actor (_container, _actor);                 \
      } } G_STMT_END

  G_END_DECLS

#endif /* __CHAMPLAIN_GROUP_H__ */
