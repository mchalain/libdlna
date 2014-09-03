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

#ifndef DIDL_H
#define DIDL_H

#include "avts.h"

void
didl_add_header (struct buffer_s *out);
void
didl_add_footer (struct buffer_s *out);
int
didl_add_tag (struct buffer_s *out, char *tag, char *value);
void
didl_add_param (struct buffer_s *out, char *param, char *value);
void
didl_add_value (struct buffer_s *out, char *param, off_t value);
void
didl_add_short_item (buffer_t *out,
    uint32_t id, dlna_item_t *item, uint32_t containerid);
void
didl_add_item (struct buffer_s *out,  
    uint32_t id, dlna_item_t *item, uint32_t containerid,
    char *restricted, char *filter, char *protocol_info);
void
didl_add_container (struct buffer_s *out, struct vfs_item_s *item,
                    char *restricted, char *searchable, char *class);

#endif
