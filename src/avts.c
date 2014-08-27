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

/*
 * AVTransport service specifications can be found at:
 * http://upnp.org/standardizeddcps/documents/AVTransport1.0.pdf
 * http://www.upnp.org/specs/av/UPnP-av-AVTransport-v2-Service-20060531.pdf
 */

#include <stdlib.h>

#include "upnp_internals.h"
#include "avts.h"

#define AVTS_ERR_ACTION_FAILED                 501

extern uint32_t
crc32(uint32_t crc, const void *buf, size_t size);

static dlna_dmp_item_t *
playlist_empty (dlna_dmp_item_t *playlist)
{
  dlna_dmp_item_t *item;

  for (item = playlist; item; item = item->hh.next)
    HASH_DEL (playlist, item);
  return playlist;
}

static dlna_dmp_item_t *
playlist_add_item (dlna_dmp_item_t *playlist, dlna_t *dlna, char *uri, char *uri_metadata dlna_unused)
{
  dlna_dmp_item_t *item;

  item = calloc (1, sizeof(dlna_dmp_item_t));
  item->item = dlna_item_new (dlna, uri);
  /* set id with the id of the last item + 1 */
  item->id = crc32(0, uri, strlen(uri));
  HASH_ADD_INT (playlist, id, item);

  return playlist;
}

static dlna_dmp_item_t *
playlist_next (dlna_dmp_item_t *playlist, uint32_t id)
{
  dlna_dmp_item_t *item = NULL;
  if (id)
  {
    HASH_FIND_INT (playlist, &id, item);
    if (item)
      return item;
  }
  return playlist;
}

static int
playitem_prepare (dlna_item_t *item dlna_unused)
{
  return 0;
}

static int
playitem_decodeframe (dlna_item_t *item dlna_unused)
{
  return 0;
}

static void *
avts_thread_play (void *arg)
{
  dlna_dmp_t *instance = (dlna_dmp_t *) arg;
  dlna_dmp_item_t *item;

  item = playlist_next (instance->playlist, 0);
  if (!item)
    return NULL;
  playitem_prepare (item->item);
  while (1)
  {
    int play_frame = 0;

    ithread_mutex_lock (&instance->state_mutex);
    switch (instance->state)
    {
    case E_STOPPED:
      ithread_mutex_unlock (&instance->state_mutex);
      return NULL;
    case E_PLAYING:
      ithread_mutex_unlock (&instance->state_mutex);
      play_frame = 1;
      break;
    case E_PAUSING:
      ithread_cond_wait (&instance->state_change, &instance->state_mutex);
      ithread_mutex_unlock (&instance->state_mutex);
      break;
    }
    if (play_frame)
    {
      if (playitem_decodeframe (item->item))
      {
        item = playlist_next (instance->playlist, item->id);
        if (!item)
        {
          ithread_mutex_lock (&instance->state_mutex);
          instance->state = E_STOPPED;
          ithread_cond_signal (&instance->state_change);
          ithread_mutex_unlock (&instance->state_mutex);
        }
        else
          playitem_prepare (item->item);
      }
    }
  }
  return NULL;
}

static dlna_dmp_t *
avts_set_thread (dlna_t *dlna, uint32_t id)
{
  dlna_dmp_t *instance = NULL;

  instance = calloc (1, sizeof(dlna_dmp_t));

  ithread_mutex_init (&instance->state_mutex, NULL);
  ithread_cond_init (&instance->state_change, NULL);
  instance->state = E_PAUSING;
  instance->id = id;
  HASH_ADD_INT (dlna->dmp, id, instance);
  ithread_create (&instance->playthread, NULL, avts_thread_play, instance);
  return instance;
}

static int
avts_set_uri (dlna_t *dlna, upnp_action_event_t *ev)
{
  char *uri, *uri_metadata;
  uint32_t InstanceID;
  buffer_t *out = NULL;
  dlna_dmp_t *instance = NULL;

  if (!dlna || !ev)
  {
    ev->ar->ErrCode = AVTS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Check for status */
  if (!ev->status)
  {
    ev->ar->ErrCode = AVTS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Retrieve input arguments */
  InstanceID   = upnp_get_ui4 (ev->ar, AVTS_ARG_INSTANCEID);
  uri   = upnp_get_string (ev->ar, AVTS_ARG_CURRENT_URI);
  uri_metadata = upnp_get_string (ev->ar, AVTS_ARG_CURRENT_URI_METADATA);

  HASH_FIND_INT (dlna->dmp, &InstanceID, instance);
  if (!instance)
  {
    instance = avts_set_thread (dlna, InstanceID);
  }
  if (instance->state == E_STOPPED)
  {
    instance->playlist = playlist_empty (instance->playlist);
  }
  instance->playlist = playlist_add_item (instance->playlist, dlna, uri, uri_metadata);

  out = buffer_new ();
  buffer_free (out);
  free (uri);
  free (uri_metadata);

  return 0;
}

static int
avts_set_next_uri (dlna_t *dlna, upnp_action_event_t *ev)
{
  char *uri, *uri_metadata;
  uint32_t instanceID;
  buffer_t *out = NULL;
  dlna_dmp_t *instance = NULL;

  if (!dlna || !ev)
  {
    ev->ar->ErrCode = AVTS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Check for status */
  if (!ev->status)
  {
    ev->ar->ErrCode = AVTS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Retrieve input arguments */
  instanceID   = upnp_get_ui4 (ev->ar, AVTS_ARG_INSTANCEID);
  uri   = upnp_get_string (ev->ar, AVTS_ARG_NEXT_URI);
  uri_metadata = upnp_get_string (ev->ar, AVTS_ARG_NEXT_URI_METADATA);

  HASH_FIND_INT (dlna->dmp, &instanceID, instance);
  playlist_add_item (instance->playlist, dlna, uri, uri_metadata);

  out = buffer_new ();
  buffer_free (out);
  free (uri);
  free (uri_metadata);

  return 0;
}

static int
avts_play (dlna_t *dlna, upnp_action_event_t *ev)
{
  int speed;
  uint32_t InstanceID;
  buffer_t *out = NULL;
  dlna_dmp_t *instance = NULL;

  if (!dlna || !ev)
  {
    ev->ar->ErrCode = AVTS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Check for status */
  if (!ev->status)
  {
    ev->ar->ErrCode = AVTS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Retrieve input arguments */
  InstanceID   = upnp_get_ui4 (ev->ar, AVTS_ARG_INSTANCEID);
  speed = upnp_get_ui4 (ev->ar, AVTS_ARG_SPEED);

  HASH_FIND_INT (dlna->dmp, &InstanceID, instance);
  ithread_mutex_lock (&instance->state_mutex);
  instance->state = E_PLAYING;
  ithread_cond_signal (&instance->state_change);
  ithread_mutex_unlock (&instance->state_mutex);

  out = buffer_new ();
  buffer_free (out);

  return 0;
}

static int
avts_stop (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t InstanceID;
  buffer_t *out = NULL;
  dlna_dmp_t *instance = NULL;

  if (!dlna || !ev)
  {
    ev->ar->ErrCode = AVTS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Check for status */
  if (!ev->status)
  {
    ev->ar->ErrCode = AVTS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Retrieve input arguments */
  InstanceID   = upnp_get_ui4 (ev->ar, AVTS_ARG_INSTANCEID);

  HASH_FIND_INT (dlna->dmp, &InstanceID, instance);
  ithread_mutex_lock (&instance->state_mutex);
  instance->state = E_STOPPED;
  ithread_cond_signal (&instance->state_change);
  ithread_mutex_unlock (&instance->state_mutex);
  ithread_join (instance->playthread, NULL);
  HASH_DEL (dlna->dmp, instance);
  out = buffer_new ();
  buffer_free (out);

  return 0;
}

static int
avts_pause (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t InstanceID;
  buffer_t *out = NULL;
  dlna_dmp_t *instance = NULL;

  if (!dlna || !ev)
  {
    ev->ar->ErrCode = AVTS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Check for status */
  if (!ev->status)
  {
    ev->ar->ErrCode = AVTS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Retrieve input arguments */
  InstanceID   = upnp_get_ui4 (ev->ar, AVTS_ARG_INSTANCEID);

  HASH_FIND_INT (dlna->dmp, &InstanceID, instance);
  ithread_mutex_lock (&instance->state_mutex);
  if (instance->state == E_PLAYING)
  {
    instance->state = E_PAUSING;
    ithread_cond_signal (&instance->state_change);
  }
  ithread_mutex_unlock (&instance->state_mutex);

  out = buffer_new ();
  buffer_free (out);

  return 0;
}

static int
avts_next (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t InstanceID;
  buffer_t *out = NULL;
  dlna_dmp_t *instance = NULL;

  if (!dlna || !ev)
  {
    ev->ar->ErrCode = AVTS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Check for status */
  if (!ev->status)
  {
    ev->ar->ErrCode = AVTS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Retrieve input arguments */
  InstanceID   = upnp_get_ui4 (ev->ar, AVTS_ARG_INSTANCEID);

  HASH_FIND_INT (dlna->dmp, &InstanceID, instance);

  out = buffer_new ();
  buffer_free (out);

  return 0;
}

static int
avts_previous (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t InstanceID;
  buffer_t *out = NULL;
  dlna_dmp_t *instance = NULL;

  if (!dlna || !ev)
  {
    ev->ar->ErrCode = AVTS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Check for status */
  if (!ev->status)
  {
    ev->ar->ErrCode = AVTS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Retrieve input arguments */
  InstanceID   = upnp_get_ui4 (ev->ar, AVTS_ARG_INSTANCEID);

  HASH_FIND_INT (dlna->dmp, &InstanceID, instance);

  out = buffer_new ();
  buffer_free (out);

  return 0;
}

/* List of UPnP AVTransport Service actions */
upnp_service_action_t avts_service_actions[] = {
  { AVTS_ACTION_SET_URI,           avts_set_uri },
  { AVTS_ACTION_SET_NEXT_URI,      avts_set_next_uri },
  { AVTS_ACTION_GET_MEDIA_INFO,    NULL },
  { AVTS_ACTION_GET_INFO,          NULL },
  { AVTS_ACTION_GET_POS_INFO,      NULL },
  { AVTS_ACTION_GET_CAPS,          NULL },
  { AVTS_ACTION_GET_SETTINGS,      NULL },
  { AVTS_ACTION_STOP,              avts_stop },
  { AVTS_ACTION_PLAY,              avts_play },
  { AVTS_ACTION_PAUSE,             avts_pause },
  { AVTS_ACTION_RECORD,            NULL },
  { AVTS_ACTION_SEEK,              NULL },
  { AVTS_ACTION_NEXT,              avts_next },
  { AVTS_ACTION_PREVIOUS,          avts_previous },
  { AVTS_ACTION_SET_PLAY_MODE,     NULL },
  { AVTS_ACTION_SET_RECORD_MODE,   NULL },
  { AVTS_ACTION_GET_ACTIONS,       NULL },
  { NULL,                                  NULL }
};
