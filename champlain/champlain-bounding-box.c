/*
 * Copyright (C) 2009 Simon Wenner <simon@wenner.ch>
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
 * SECTION:champlain-bounding-box
 * @short_description: A basic struct to describe a bounding box
 */

#include "champlain-bounding-box.h"

GType
champlain_bounding_box_get_type (void)
{
  static GType type = 0;

  if (G_UNLIKELY (type == 0))
    {
      type = g_boxed_type_register_static (g_intern_static_string ("ChamplainBoundingBox"),
          (GBoxedCopyFunc) champlain_bounding_box_copy,
          (GBoxedFreeFunc) champlain_bounding_box_free);
    }

  return type;
}

/**
 * champlain_bounding_box_copy:
 * @bbox: a #ChamplainBoundingBox
 *
 * Makes a copy of the bounding box structure. The result must be
 * freed using champlain_bounding_box_free().
 *
 * Return value: an allocated copy of @bbox.
 *
 * Since: 0.6
 */
ChamplainBoundingBox *
champlain_bounding_box_copy (const ChamplainBoundingBox *bbox)
{
  if (G_LIKELY (bbox != NULL))
    return g_slice_dup (ChamplainBoundingBox, bbox);

  return NULL;
}

/**
 * champlain_bounding_box_free:
 * @bbox: a #ChamplainBoundingBox
 *
 * Frees a bounding box structure created with #champlain_bounding_box_new or
 * #champlain_bounding_box_copy
 *
 * Since: 0.6
 */
void
champlain_bounding_box_free (ChamplainBoundingBox *bbox)
{

  if (G_UNLIKELY (bbox == NULL))
    return;

  g_slice_free (ChamplainBoundingBox, bbox);
}

/**
 * champlain_bounding_box_new:
 *
 * Return value: a newly allocated #ChamplainBoundingBox to be freed
 * with #champlain_bounding_box_free
 *
 * Since: 0.6
 */
ChamplainBoundingBox *
champlain_bounding_box_new ()
{
  return g_slice_new (ChamplainBoundingBox);
}
