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

typedef struct dlna_stream_s dlna_stream_t;
typedef struct dlna_vfs_s dlna_vfs_t;
typedef struct vfs_item_s vfs_item_t;
typedef struct dlna_item_s dlna_item_t;

/* UPnP Services */
typedef struct upnp_action_event_s    upnp_action_event_t;
typedef struct upnp_service_statevar_s  upnp_service_statevar_t;
typedef struct upnp_service_action_arg_s  upnp_service_action_arg_t;
typedef struct upnp_service_action_s  upnp_service_action_t;

struct upnp_action_event_s {
  struct dlna_Action_Request *ar;
  int status;
  dlna_service_t *service;
  dlna_device_t *device;
};

struct upnp_service_action_arg_s {
  char *name;
  enum {
    E_INPUT,
    E_OUTPUT,
  } dir;
  upnp_service_statevar_t *relation;
};

struct upnp_service_action_s {
  char *name;
  char *args;
  upnp_service_action_arg_t **args_s;
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
  char **allowed;
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
  /* defines capability mode */
  dlna_capability_mode_t mode;
  /* defines flexibility on file extension's check */
  int check_extensions;

  /* Eventing mechanism */
  ithread_mutex_t event_mutex;
  ithread_cond_t eventing;
  ithread_t event_thread;

  /* Profilers entries */
  dlna_profiler_list_t *profilers;

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
 * DLNA Media Object item
 */
struct dlna_item_s {
  char *filename;
  int64_t filesize;
  char *profileid;
  dlna_properties_t *properties;
  dlna_metadata_t *metadata;
  const dlna_profile_t *profile;
  dlna_stream_t *stream;
  void *profile_cookie;
};

dlna_stream_t *stream_open (char *url);
void stream_close (dlna_stream_t *stream);

/* defines verbosity level */
extern  dlna_verbosity_level_t dlna_verbosity;
void dlna_log (dlna_verbosity_level_t level,
               const char *format, ...);

const dlna_profile_t *dlna_get_media_profile_by_id (dlna_t *dlna, char *profileid);
char * dlna_upnp_object_type (dlna_media_class_t media_class);

void dlna_http_set_callback (const char *virtualdir, dlna_http_callback_t *cb);

extern dlna_profiler_t upnpav_profiler;

typedef struct protocol_info_s protocol_info_t;
struct protocol_info_s
{
  dlna_protocol_t *protocol;
  const dlna_profile_t *profile;
  char *(*other) (protocol_info_t *pinfo);
  protocol_info_t *next;
};

#endif /* DLNA_INTERNALS_H */
