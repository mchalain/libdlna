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
#include "threadutil/ithread.h"
#else
#include <upnp.h>
#include <upnptools.h>
#include <ithread.h>
#include "upnp_dlna_wrapper.h"
#endif

#include "uthash.h"

/* UPnP Service properties */
typedef struct dlna_vfs_s dlna_vfs_t;
struct dlna_vfs_s
{
  /* VFS for Content Directory */
  dlna_dms_storage_type_t storage_type;
  struct vfs_item_s *vfs_root;
  uint32_t vfs_items;
#ifdef HAVE_SQLITE
  void *db;
#endif /* HAVE_SQLITE */
};

/* UPnP Services */
typedef struct upnp_action_event_s    upnp_action_event_t;
typedef struct upnp_service_statevar_s  upnp_service_statevar_t;
typedef struct upnp_service_action_s  upnp_service_action_t;

struct upnp_action_event_s {
  struct dlna_Action_Request *ar;
  int status;
  dlna_service_t *service;
  dlna_device_t *device;
};

struct upnp_service_action_s {
  char *name;
  char *args;
  int (*cb) (dlna_t *, upnp_action_event_t *);
};

struct upnp_service_statevar_s {
  char *name;
  enum {
    E_STRING,
    E_BOOLEAN,
    E_I2,
    E_UI2,
    E_I4,
    E_UI4,
    E_URI,
  } type;
  uint32_t eventing;
  char * (*get) (dlna_t *, dlna_service_t *);
};

typedef struct dlna_profiler_list_s dlna_profiler_list_t;
struct dlna_profiler_list_s
{
/**
 * pointer to the profiler object
 **/
  const dlna_profiler_t *profiler;
/**
 * pointer to the next profiler in the list
 **/
  struct dlna_profiler_list_s *next;
};

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
  /* DLNA flags*/
  int flags;

  /* Internal HTTP Server */
  dlna_http_callback_t *http_callback;

  /* Eventing mechanism */
  ithread_mutex_t event_mutex;
  ithread_cond_t eventing;
  ithread_t event_thread;

  /* Profilers entries */
  dlna_profiler_list_t *profilers;

  /* DMS Properties */
  dlna_vfs_t dms;

  /* cms Service data */
  struct {
    char **sourcemimes;
    char **sinkmimes;
  } cms;
  /* UPnP Properties */
  char *interface;
  unsigned short port; /* server port */
  dlnaDevice_Handle dev;
  struct dlna_device_s *device;
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
  void *profile_cookie;
};

/**
 * Return the DLNA media object item.
 *
 * @param[in] dlna     The DLNA library's controller.
 * @param[in] item     The VFS item corresponding to the file.
 * @return The DLNA object item if existing, NULL otherwise.
 */
dlna_item_t *
dlna_item_get(dlna_t *dlna, struct vfs_item_s *item);

void dlna_log (dlna_t *dlna,
               dlna_verbosity_level_t level,
               const char *format, ...);
char **dlna_get_supported_mime_types (dlna_t *dlna);

dlna_profile_t *dlna_get_media_profile (dlna_t *dlna, char *profileid);

void dlna_append_supported_mime_types (dlna_t *dlna, int sink, char *mime);


extern dlna_profiler_t upnpav_profiler;

#endif /* DLNA_INTERNALS_H */
