/*
 * Copyright (C) 2008-2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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

#ifndef CHAMPLAIN_PRIVATE_H
#define CHAMPLAIN_PRIVATE_H

#include <glib.h>
#include <clutter/clutter.h>

typedef struct _Map Map;

typedef struct
{
  gfloat x;
  gfloat y;
  gfloat z;
} ChamplainFloatPoint;

struct _ChamplainBaseMarkerPrivate
{
  gdouble lon;
  gdouble lat;
  gboolean highlighted;
};

typedef struct
{
  gfloat x;
  gfloat y;
  gint width;
  gint height;
} ChamplainRectangle;

#endif
