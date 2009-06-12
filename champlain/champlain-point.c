/*
 * Copyright (C) 2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
 *
 * This file is inspired by clutter-color.c which is
 * Copyright (C) 2006 OpenedHand, and has the same license.
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
 * SECTION:champlain-point
 * @short_description: A base struct to store a latitude and longitude
 */

#include "champlain-point.h"

GType
champlain_point_get_type (void)
{
  static GType type = 0;

  if (G_UNLIKELY (type == 0))
    {
      type = g_boxed_type_register_static (g_intern_static_string ("ChamplainPoint"),
          (GBoxedCopyFunc) champlain_point_copy,
          (GBoxedFreeFunc) champlain_point_free);
    }

  return type;
}

/**
 * champlain_point_copy:
 * @point: a #ChamplainPoint
 *
 * Makes a copy of the point structure.  The result must be
 * freed using champlain_point_free().
 *
 * Return value: an allocated copy of @point.
 *
 * Since: 0.4
 */
ChamplainPoint *
champlain_point_copy (const ChamplainPoint *point)
{
  if (G_LIKELY (point != NULL))
    return g_slice_dup (ChamplainPoint, point);

  return NULL;
}

/**
 * champlain_point_free:
 * @point: a #ChamplainPoint
 *
 * Frees a point structure created with #champlain_point_new or
 * #champlain_point_copy
 *
 * Since: 0.4
 */
void
champlain_point_free (ChamplainPoint *point)
{
  if (G_LIKELY (point != NULL))
    g_slice_free (ChamplainPoint, point);
}

/**
 * champlain_point_new:
 * @lat: the latitude
 * @lon: the longitude
 *
 * Return value: a newly allocated #ChamplainPoint to be freed with #champlain_point_free
 *
 * Since: 0.4
 */
ChamplainPoint *
champlain_point_new (gdouble lat, gdouble lon)
{
  ChamplainPoint *point;

  point = g_slice_new (ChamplainPoint);

  point->lat = lat;
  point->lon = lon;

  return point;
}
