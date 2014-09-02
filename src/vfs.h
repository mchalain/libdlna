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

#ifndef VFS_H
#define VFS_H

typedef struct vfs_item_s {
  uint32_t id;
  char *title;

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
      struct vfs_item_s **children;
      uint32_t children_count;
      uint32_t updateID; /* UPnP/AV ContentDirectory v2 Service ch 2.2.9*/
    } container;
  } u;

  struct vfs_item_s *parent;

  UT_hash_handle hh;
} vfs_item_t;

typedef struct vfs_s vfs_t;
struct vfs_s
{
  int mode;
  /* VFS for Content Directory */
  dlna_dms_storage_type_t storage_type;
  struct vfs_item_s *vfs_root;
  uint32_t vfs_items;
#ifdef HAVE_SQLITE
  void *db;
#endif /* HAVE_SQLITE */
};

/***************************************************************************/
/*                                                                         */
/* DLNA UPnP Virtual File System (VFS) Management                          */
/*  Optional: Routines to add/remove element from VFS.                     */
/*                                                                         */
/***************************************************************************/

/**
 * Add a new container to the VFS layer.
 *
 * @param[in] vfs         The DLNA library's controller.
 * @param[in] name         Displayed name of the container.
 * @param[in] object_id    Expected UPnP object ID.
 * @param[in] container_id UPnP object ID of its parent.
 * @return The attrbiuted UPnP object ID if successfull, 0 otherwise.
 */
uint32_t dlna_vfs_add_container (dlna_t *dlna, char *name,
                        uint32_t object_id, uint32_t container_id);

/**
 * Add a new resource to the VFS layer.
 *
 * @param[in] vfs         The DLNA library's controller.
 * @param[in] name         Displayed name of the resource.
 * @param[in] fullname     Full path to the specified resource.
 * @param[in] container_id UPnP object ID of its parent.
 * @return The attrbiuted UPnP object ID if successfull, 0 otherwise.
 */
uint32_t dlna_vfs_add_resource (dlna_t *dlna, char *name,
                       char *fullpath, uint32_t container_id);

vfs_item_t *vfs_get_item_by_id (dlna_t *dlna, uint32_t id);
vfs_item_t *vfs_get_item_by_name (dlna_t *dlna, char *name);
void vfs_item_free (dlna_t *dlna, vfs_item_t *item);

#endif
