/* champlain-finger-scroll.h: Finger scrolling container actor
 *
 * Copyright (C) 2008 OpenedHand
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Chris Lord <chris@openedhand.com>
 */

#ifndef __CHAMPLAIN_FINGER_SCROLL_H__
#define __CHAMPLAIN_FINGER_SCROLL_H__

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_FINGER_SCROLL            (champlain_finger_scroll_get_type())
#define CHAMPLAIN_FINGER_SCROLL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_FINGER_SCROLL, ChamplainFingerScroll))
#define CHAMPLAIN_IS_FINGER_SCROLL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_FINGER_SCROLL))
#define CHAMPLAIN_FINGER_SCROLL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_FINGER_SCROLL, ChamplainFingerScrollClass))
#define CHAMPLAIN_IS_FINGER_SCROLL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_FINGER_SCROLL))
#define CHAMPLAIN_FINGER_SCROLL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_FINGER_SCROLL, ChamplainFingerScrollClass))


typedef struct _ChamplainFingerScroll          ChamplainFingerScroll;
typedef struct _ChamplainFingerScrollPrivate   ChamplainFingerScrollPrivate;
typedef struct _ChamplainFingerScrollClass     ChamplainFingerScrollClass;

struct _ChamplainFingerScroll
{
  /*< private >*/
  ClutterActor parent_instance;
  
  ChamplainFingerScrollPrivate *priv;
};

struct _ChamplainFingerScrollClass
{
  ClutterActorClass parent_class;
};

GType champlain_finger_scroll_get_type (void) G_GNUC_CONST;

ClutterActor *champlain_finger_scroll_new  (gboolean kinetic);

void          champlain_finger_scroll_stop (ChamplainFingerScroll *scroll);

G_END_DECLS

#endif /* __CHAMPLAIN_FINGER_SCROLL_H__ */
