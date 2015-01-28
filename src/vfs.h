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

typedef struct vfs_item_s vfs_item_t;
typedef struct vfs_resource_s vfs_resource_t;

typedef struct
{
  dlna_protocol_info_type_t protocolid:8;
  dlna_org_operation_t op:8;
  dlna_org_play_speed_t speed:4;
  dlna_org_conversion_t cnv:2;
} vfs_item_info_t;

struct vfs_resource_s {
  char *(*url) (vfs_resource_t *resource);
  protocol_info_t *protocol_info;
  void *cookie;
  int64_t size;
  vfs_item_info_t info;
  dlna_properties_t properties;
  struct vfs_resource_s *next;
};

typedef struct vfs_items_list_s vfs_items_list_t;

struct vfs_items_list_s {
  vfs_item_t *item;
  vfs_items_list_t *next;
  vfs_items_list_t *previous;
};

struct vfs_item_s {
  uint32_t id;
  dlna_restricted_t restricted;

  enum {
    DLNA_RESOURCE,
    DLNA_CONTAINER
  } type;

  union {
    struct {
      dlna_item_t *item;
      vfs_resource_t *resources;        
    } resource;
    struct {
      char *title;
      vfs_items_list_t *(*children) (vfs_item_t *item);
      void (*add_child) (vfs_item_t *container, vfs_item_t *child);
      uint32_t children_count;
      uint32_t updateID; /* UPnP/AV ContentDirectory v2 Service ch 2.2.9*/
      dlna_media_class_t media_class;
      void *children_cookie;
    } container;
  } u;

  struct vfs_item_s *parent;
  struct vfs_item_s *root;

  UT_hash_handle hh;
};

struct dlna_protocol_s
{
  dlna_protocol_info_type_t type;
  vfs_resource_t *(*create_resource)(vfs_item_t *item);
  const char *(*name)();
  const char *(*net)();
  void *cookie;
  dlna_protocol_t *next;
};

struct dlna_vfs_s
{
  /* VFS for Content Directory */
  dlna_dms_storage_type_t storage_type;
  struct vfs_item_s *vfs_root;
  dlna_protocol_t *protocols;
  protocol_info_t *sources;
  uint32_t vfs_items;
  /* DLNA flags*/
  dlna_org_flags_t flags;
  int mode;
};

vfs_item_t *vfs_get_item_by_id (dlna_vfs_t *vfs, uint32_t id);
dlna_item_t *vfs_item_get(vfs_item_t *item);
inline vfs_resource_t *vfs_resource_get (vfs_item_t *item);

typedef struct didl_result_s
{
	void *didl;
	unsigned long updateid;
	unsigned long nb_returned;
	unsigned long total_match;
} didl_result_t;

int
vfs_browse_metadata (vfs_item_t *item, char *filter, didl_result_t *result);
int
vfs_browse_directchildren (vfs_item_t *item, 
                           int index, unsigned long count, 
                           char *filter, char *sort,
                           didl_result_t *result);
int
vfs_search_directchildren(vfs_item_t *item, int index,
                           uint32_t count, char *filter,
                           char *search_criteria, didl_result_t *result);

#endif
