#include "champlain-perl.h"


MODULE = Champlain::Cache  PACKAGE = Champlain::Cache  PREFIX = champlain_cache_


ChamplainCache*
champlain_cache_dup_default (class)
	C_ARGS: /* No args */


void
champlain_cache_update_tile (ChamplainCache *self, ChamplainTile *tile, guint filesize)


gboolean
champlain_cache_fill_tile (ChamplainCache *self, ChamplainTile *tile)


gboolean
champlain_cache_tile_is_expired (ChamplainCache *self, ChamplainTile *tile)


void
champlain_cache_set_size_limit (ChamplainCache *self, guint size_limit)


guint
champlain_cache_get_size_limit (ChamplainCache *self)


void
champlain_cache_purge (ChamplainCache *self)


void
champlain_cache_purge_on_idle (ChamplainCache *self)
