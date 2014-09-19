/*
 * libdlna: reference DLNA standards implementation.
 * Copyright (C) 2007-2008 Benjamin Zores <ben@geexbox.org>
 * Copyright (C) 2014-2016 Marc Chalain <marc.chalain@gmail.com>
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

#ifndef VFS_H
#define VFS_H

typedef struct vfs_item_s {
  uint32_t id;

  enum {
    DLNA_RESOURCE,
    DLNA_CONTAINER
  } type;

  union {
    struct {
      dlna_item_t *item;
      dlna_org_conversion_t cnv;
      char *fullpath;
      char *url;
      int fd;
    } resource;
    struct {
      char *title;
      struct vfs_item_s **children;
      uint32_t children_count;
      uint32_t updateID; /* UPnP/AV ContentDirectory v2 Service ch 2.2.9*/
    } container;
  } u;

  struct vfs_item_s *parent;

  UT_hash_handle hh;
} vfs_item_t;

struct dlna_vfs_s
{
  /* VFS for Content Directory */
  dlna_dms_storage_type_t storage_type;
  struct vfs_item_s *vfs_root;
  uint32_t vfs_items;
  int mode;
};

vfs_item_t *vfs_get_item_by_id (dlna_vfs_t *vfs, uint32_t id);
vfs_item_t *vfs_get_item_by_name (dlna_vfs_t *vfs, char *name);
void vfs_item_free (dlna_vfs_t *vfs, vfs_item_t *item);

#endif
