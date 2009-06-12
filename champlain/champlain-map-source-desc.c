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
 * SECTION:champlain-map-source-desc
 * @short_description: A basic struct to describe map sources
 */

#include "champlain-map-source-desc.h"

GType
champlain_map_source_desc_get_type (void)
{
  static GType type = 0;

  if (G_UNLIKELY (type == 0))
    {
      type = g_boxed_type_register_static (g_intern_static_string ("ChamplainMapSourceDesc"),
          (GBoxedCopyFunc) champlain_map_source_desc_copy,
          (GBoxedFreeFunc) champlain_map_source_desc_free);
    }

  return type;
}

/**
 * champlain_map_source_desc_copy:
 * @desc: a #ChamplainMapSourceDesc
 *
 * Makes a copy of the map source desc structure.  The result must be
 * freed using champlain_map_source_desc_free().
 *
 * Return value: an allocated copy of @desc.
 *
 * Since: 0.4
 */
ChamplainMapSourceDesc *
champlain_map_source_desc_copy (const ChamplainMapSourceDesc *desc)
{
  if (G_LIKELY (desc != NULL))
    return g_slice_dup (ChamplainMapSourceDesc, desc);

  return NULL;
}

/**
 * champlain_map_source_desc_free:
 * @desc: a #ChamplainMapSourceDesc
 *
 * Frees a desc structure created with #champlain_map_source_desc_new or
 * #champlain_map_source_desc_copy
 *
 * Since: 0.4
 */
void
champlain_map_source_desc_free (ChamplainMapSourceDesc *desc)
{

  if (G_UNLIKELY (desc == NULL))
    return;

  if (G_LIKELY (desc->id != NULL))
    g_free (desc->id);

  g_slice_free (ChamplainMapSourceDesc, desc);
}

/**
 * champlain_map_source_desc_new:
 * @lat: the latitude
 * @lon: the longitude
 *
 * Return value: a newly allocated #ChamplainMapSourceDesc to be freed with #champlain_map_source_desc_free
 *
 * Since: 0.4
 */
ChamplainMapSourceDesc *
champlain_map_source_desc_new ()
{
  return g_slice_new (ChamplainMapSourceDesc);
}
