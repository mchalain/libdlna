/*
 * libdlna: reference DLNA standards implementation.
 * Copyright (C) 2007-2008 Benjamin Zores <ben@geexbox.org>
 *
 * This file is part of libdlna.
 *
 * libdlna is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libdlna is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libdlna; if not, write to the Free Software
 * Foundation, Inc, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dlna_internals.h"
#include "upnp_internals.h"
#include "dlna_db.h"

static void
dms_set_memory (dlna_t *dlna)
{
  if (!dlna)
    return;

  dlna->storage_type = DLNA_DMS_STORAGE_MEMORY;
  dlna_log (dlna, DLNA_MSG_INFO, "Use memory for VFS metadata storage.\n");
}

void
dlna_dms_set_vfs_storage_type (dlna_t *dlna,
                               dlna_dms_storage_type_t type, char *data)
{
  if (!dlna)
    return;

  if (type == DLNA_DMS_STORAGE_MEMORY)
    dms_set_memory (dlna);
  else if (type == DLNA_DMS_STORAGE_SQL_DB)
  {
    if (dms_db_open (dlna, data))
    {
      data = NULL;
      dms_set_memory (dlna);
      dlna_log (dlna, DLNA_MSG_WARNING,
              "SQLite support is disabled. " \
              "Failing back to Memory based VFS storage.\n");
      return;
    }
    else if (dms_db_check (dlna))
    {
      if (dms_db_create(dlna))
      {
        dms_set_memory (dlna);
        dlna_log (dlna, DLNA_MSG_WARNING,
              "SQLite support is disabled. " \
              "Failing back to Memory based VFS storage.\n");
        return;
      }
	}
  }
}
