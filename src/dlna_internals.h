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

#ifndef DLNA_INTERNALS_H
#define DLNA_INTERNALS_H

#if defined(__GNUC__)
#    define dlna_unused __attribute__((unused))
#else
#    define dlna_unused
#endif

#include "dlna.h"

#ifndef HAVE_EXTERNAL_LIBUPNP
#include "upnp/upnp.h"
#include "upnp/upnptools.h"
#else
#include <upnp.h>
#include <upnptools.h>
#include "upnp_dlna_wrapper.h"
#endif

#include "uthash.h"

typedef struct dlna_item_s dlna_item_t;

typedef enum {
  DLNA_DEVICE_UNKNOWN,
  DLNA_DEVICE_DMS,      /* Digital Media Server */
  DLNA_DEVICE_DMP,      /* Digital Media Player */
} dlna_device_type_t;

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

vfs_item_t *vfs_get_item_by_id (dlna_t *dlna, uint32_t id);
vfs_item_t *vfs_get_item_by_name (dlna_t *dlna, char *name);
void vfs_item_free (dlna_t *dlna, vfs_item_t *item);

typedef struct upnp_service_s         upnp_service_t;
typedef struct upnp_action_event_s    upnp_action_event_t;
typedef struct upnp_service_action_s  upnp_service_action_t;

struct upnp_action_event_s {
  struct dlna_Action_Request *ar;
  int status;
  upnp_service_t *service;
};

struct upnp_service_action_s {
  char *name;
  int (*cb) (dlna_t *, upnp_action_event_t *);
};

struct upnp_service_s {
  char *id;
  char *type;
  char *scpd_url;
  char *control_url;
  char *event_url;
  upnp_service_action_t *actions;
  UT_hash_handle hh;
};

upnp_service_t *dlna_service_find (dlna_t *dlna, char *id);
void dlna_service_unregister (dlna_t *dlna, dlna_service_type_t srv);
void dlna_service_unregister_all (dlna_t *dlna);

/**
 * DLNA Library's controller.
 * This controls the whole library.
 */
struct dlna_s {
  /* has the library's been inited */
  int inited;
  /* defines verbosity level */
  dlna_verbosity_level_t verbosity;
  /* defines capability mode */
  dlna_capability_mode_t mode;
  /* defines flexibility on file extension's check */
  int check_extensions;
  /* linked-list of registered DLNA profiles */
  void *first_profile;
  /* DLNA flags*/
  int flags;

  /* Internal HTTP Server */
  dlna_http_callback_t *http_callback;

  /* UPnP Services */
  upnp_service_t *services;
  
  /* VFS for Content Directory */
  dlna_dms_storage_type_t storage_type;
  vfs_item_t *vfs_root;
  uint32_t vfs_items;
  void *db;
  
  /* UPnP Properties */
  char *interface;
  unsigned short port; /* server port */
  dlnaDevice_Handle dev;
  char *friendly_name;
  char *manufacturer;
  char *manufacturer_url;
  char *model_description;
  char *model_name;
  char *model_number;
  char *model_url;
  char *serial_number;
  char *uuid;
  char *presentation_url;
};

/***************************************************************************/
/*                                                                         */
/* DLNA Item Handling                                                      */
/*  Optional: Used to create a DLNA Media Item instance from a given file. */
/*                                                                         */
/***************************************************************************/

/**
 * DLNA Media Object item metadata
 */
typedef struct dlna_metadata_s {
  char     *title;                /* <dc:title> */
  char     *author;               /* <dc:artist> */
  char     *comment;              /* <upnp:longDescription> */
  char     *album;                /* <upnp:album> */
  uint32_t track;                 /* <upnp:originalTrackNumber> */
  char     *genre;                /* <upnp:genre> */
} dlna_metadata_t;

#define DLNA_PROPERTIES_DURATION_MAX_SIZE 64
#define DLNA_PROPERTIES_RESOLUTION_MAX_SIZE 64
/**
 * DLNA Media Object item properties
 */
typedef struct dlna_properties_s {
  char     duration[DLNA_PROPERTIES_DURATION_MAX_SIZE];          /* res@duration */
  uint32_t bitrate;               /* res@bitrate */
  uint32_t sample_frequency;      /* res@sampleFrequency */
  uint32_t bps;                   /* res@bitsPerSample */
  uint32_t channels;              /* res@nrAudioChannels */
  char     resolution[DLNA_PROPERTIES_RESOLUTION_MAX_SIZE];        /* res@resolution */
} dlna_properties_t;

/**
 * DLNA Media Object item
 */
struct dlna_item_s {
  char *filename;
  int64_t filesize;
  char *profileid;
  dlna_properties_t *properties;
  dlna_metadata_t *metadata;
  dlna_profile_t *profile;
};

/**
 * Create a new DLNA media object item.
 *
 * @param[in] dlna     The DLNA library's controller.
 * @param[in] filename The input file to be added.
 * @return A new DLNA object item if compatible, NULL otherwise.
 */
dlna_item_t *dlna_item_new (dlna_t *dlna, const char *filename);

/**
 * Free an existing DLNA media object item.
 *
 * @param[in] item     The DLNA object item to be freed.
 */
void dlna_item_free (dlna_item_t *item);

/**
 * Return the DLNA media object item.
 *
 * @param[in] dlna     The DLNA library's controller.
 * @param[in] item     The VFS item corresponding to the file.
 * @return The DLNA object item if existing, NULL otherwise.
 */
dlna_item_t *
dlna_item_get(dlna_t *dlna, vfs_item_t *item);

void dlna_log (dlna_t *dlna,
               dlna_verbosity_level_t level,
               const char *format, ...);
char **dlna_get_supported_mime_types (dlna_t *dlna);

dlna_profile_t *dlna_get_media_profile (dlna_t *dlna, char *profileid);

#endif /* DLNA_INTERNALS_H */
