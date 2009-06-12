/*
 * Copyright (C) 2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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

#if !defined (__CHAMPLAIN_CHAMPLAIN_H_INSIDE__) && !defined (CHAMPLAIN_COMPILATION)
#error "Only <champlain/champlain.h> can be included directly."
#endif

#ifndef CHAMPLAIN_POINT_H
#define CHAMPLAIN_POINT_H

#include <glib-object.h>

typedef struct _ChamplainPoint ChamplainPoint;

struct _ChamplainPoint
{
  double lat;
  double lon;
};

GType champlain_point_get_type (void) G_GNUC_CONST;
#define CHAMPLAIN_TYPE_POINT (champlain_point_get_type ())
#define CHAMPLAIN_POINT(x) ((ChamplainPoint *) (x))

ChamplainPoint * champlain_point_copy (const ChamplainPoint *point);

void champlain_point_free (ChamplainPoint *point);

ChamplainPoint * champlain_point_new (gdouble lat, gdouble lon);

#endif
