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

/* AVTS Action Names */
#define SERVICE_AVTS_ACTION_SET_URI            "SetAVTransportURI"
#define SERVICE_AVTS_ACTION_SET_NEXT_URI       "SetNextAVTransportURI"
#define SERVICE_AVTS_ACTION_GET_MEDIA_INFO     "GetMediaInfo"
#define SERVICE_AVTS_ACTION_GET_INFO           "GetTransportInfo"
#define SERVICE_AVTS_ACTION_GET_POS_INFO       "GetPositionInfo"
#define SERVICE_AVTS_ACTION_GET_CAPS           "GetDeviceCapabilities"
#define SERVICE_AVTS_ACTION_GET_SETTINGS       "GetTransportSettings"
#define SERVICE_AVTS_ACTION_STOP               "Stop"
#define SERVICE_AVTS_ACTION_PLAY               "Play"
#define SERVICE_AVTS_ACTION_PAUSE              "Pause"
#define SERVICE_AVTS_ACTION_RECORD             "Record"
#define SERVICE_AVTS_ACTION_SEEK               "Seek"
#define SERVICE_AVTS_ACTION_NEXT               "Next"
#define SERVICE_AVTS_ACTION_PREVIOUS           "Previous"
#define SERVICE_AVTS_ACTION_SET_PLAY_MODE      "SetPlayMode"
#define SERVICE_AVTS_ACTION_SET_RECORD_MODE    "SetRecordQualityMode"
#define SERVICE_AVTS_ACTION_GET_ACTIONS        "GetCurrentTransportActions"

#define SERVICE_AVTS_ARG_CURRENT_URI           "CurrentURI"
#define SERVICE_AVTS_ARG_NEXT_URI              "NextURI"
#define SERVICE_AVTS_ARG_CURRENT_URI_METADATA  "CurrentURIMetaData"
#define SERVICE_AVTS_ARG_NEXT_URI_METADATA     "NextURIMetaData"
#define SERVICE_AVTS_ARG_SPEED                 "TransportPlaySpeed"

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
playlist_add_item (dlna_dmp_item_t *playlist, dlna_t *dlna, char *uri, char *uri_metadata)
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
playitem_prepare (dlna_item_t *item)
{
  return 0;
}

static int
playitem_decodeframe (dlna_item_t *item)
{
  return 0;
}

static void *
avts_thread_play (void *arg)
{
  dlna_t *dlna = (dlna_t *) arg;
  dlna_dmp_item_t *item;

  item = playlist_next (dlna->dmp.playlist, 0);
  if (!item)
    return NULL;
  playitem_prepare (item->item);
  while (1)
  {
    int play_frame = 0;

    ithread_mutex_lock (&dlna->dmp.state_mutex);
    switch (dlna->dmp.state)
    {
    case E_STOPPED:
      ithread_mutex_unlock (&dlna->dmp.state_mutex);
      return NULL;
    case E_PLAYING:
      ithread_mutex_unlock (&dlna->dmp.state_mutex);
      play_frame = 1;
      break;
    case E_PAUSING:
      ithread_cond_wait (&dlna->dmp.state_change, &dlna->dmp.state_mutex);
      ithread_mutex_unlock (&dlna->dmp.state_mutex);
      break;
    }
    if (play_frame)
    {
      if (playitem_decodeframe (item->item))
      {
        item = playlist_next (dlna->dmp.playlist, item->id);
        if (!item)
        {
          ithread_mutex_lock (&dlna->dmp.state_mutex);
          dlna->dmp.state = E_STOPPED;
          ithread_cond_signal (&dlna->dmp.state_change);
          ithread_mutex_unlock (&dlna->dmp.state_mutex);
        }
        else
          playitem_prepare (item->item);
      }
    }
  }
}

static int
avts_set_thread (dlna_t *dlna, uint32_t id)
{
  ithread_mutex_init (&dlna->dmp.state_mutex, NULL);
  ithread_cond_init (&dlna->dmp.state_change, NULL);
  dlna->dmp.state = E_PAUSING;
  dlna->dmp.thread_id = id;
  ithread_create (&dlna->dmp.playthread, NULL, avts_thread_play, dlna);
  return 0;
}

static int
avts_set_uri (dlna_t *dlna, upnp_action_event_t *ev)
{
  char *URI, *URIMetadata;
  buffer_t *out = NULL;

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
  URI   = upnp_get_string (ev->ar, SERVICE_AVTS_ARG_CURRENT_URI);
  URIMetadata = upnp_get_string (ev->ar, SERVICE_AVTS_ARG_CURRENT_URI_METADATA);

  if (dlna->dmp.state == E_STOPPED)
  {
    dlna->dmp.playlist = playlist_empty (dlna->dmp.playlist);
    avts_set_thread (dlna, 0);
  }
  dlna->dmp.playlist = playlist_add_item (dlna->dmp.playlist, dlna, URI, URIMetadata);

  out = buffer_new ();
  buffer_free (out);
  free (URI);
  free (URIMetadata);

  return 0;
}

static int
avts_set_next_uri (dlna_t *dlna, upnp_action_event_t *ev)
{
  char *URI, *URIMetadata;
  buffer_t *out = NULL;

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
  URI   = upnp_get_string (ev->ar, SERVICE_AVTS_ARG_NEXT_URI);
  URIMetadata = upnp_get_string (ev->ar, SERVICE_AVTS_ARG_NEXT_URI_METADATA);

  playlist_add_item (dlna->dmp.playlist, dlna, URI, URIMetadata);

  out = buffer_new ();
  buffer_free (out);
  free (URI);
  free (URIMetadata);

  return 0;
}

static int
avts_play (dlna_t *dlna, upnp_action_event_t *ev)
{
  int speed;
  buffer_t *out = NULL;
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
  speed = upnp_get_ui4 (ev->ar, SERVICE_AVTS_ARG_SPEED);

  ithread_mutex_lock (&dlna->dmp.state_mutex);
  dlna->dmp.state = E_PLAYING;
  ithread_cond_signal (&dlna->dmp.state_change);
  ithread_mutex_unlock (&dlna->dmp.state_mutex);

  out = buffer_new ();
  buffer_free (out);

  return 0;
}

static int
avts_stop (dlna_t *dlna, upnp_action_event_t *ev)
{
  buffer_t *out = NULL;
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

  ithread_mutex_lock (&dlna->dmp.state_mutex);
  dlna->dmp.state = E_STOPPED;
  ithread_cond_signal (&dlna->dmp.state_change);
  ithread_mutex_unlock (&dlna->dmp.state_mutex);

  out = buffer_new ();
  buffer_free (out);

  return 0;
}

static int
avts_pause (dlna_t *dlna, upnp_action_event_t *ev)
{
  buffer_t *out = NULL;
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

  ithread_mutex_lock (&dlna->dmp.state_mutex);
  if (dlna->dmp.state == E_PLAYING)
  {
    dlna->dmp.state = E_PAUSING;
    ithread_cond_signal (&dlna->dmp.state_change);
  }
  ithread_mutex_unlock (&dlna->dmp.state_mutex);

  out = buffer_new ();
  buffer_free (out);

  return 0;
}

static int
avts_next (dlna_t *dlna, upnp_action_event_t *ev)
{
  buffer_t *out = NULL;
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

  out = buffer_new ();
  buffer_free (out);

  return 0;
}

static int
avts_previous (dlna_t *dlna, upnp_action_event_t *ev)
{
  buffer_t *out = NULL;
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

  out = buffer_new ();
  buffer_free (out);

  return 0;
}

/* List of UPnP AVTransport Service actions */
upnp_service_action_t avts_service_actions[] = {
  { SERVICE_AVTS_ACTION_SET_URI,           avts_set_uri },
  { SERVICE_AVTS_ACTION_SET_NEXT_URI,      avts_set_next_uri },
  { SERVICE_AVTS_ACTION_GET_MEDIA_INFO,    NULL },
  { SERVICE_AVTS_ACTION_GET_INFO,          NULL },
  { SERVICE_AVTS_ACTION_GET_POS_INFO,      NULL },
  { SERVICE_AVTS_ACTION_GET_CAPS,          NULL },
  { SERVICE_AVTS_ACTION_GET_SETTINGS,      NULL },
  { SERVICE_AVTS_ACTION_STOP,              avts_stop },
  { SERVICE_AVTS_ACTION_PLAY,              avts_play },
  { SERVICE_AVTS_ACTION_PAUSE,             avts_pause },
  { SERVICE_AVTS_ACTION_RECORD,            NULL },
  { SERVICE_AVTS_ACTION_SEEK,              NULL },
  { SERVICE_AVTS_ACTION_NEXT,              avts_next },
  { SERVICE_AVTS_ACTION_PREVIOUS,          avts_previous },
  { SERVICE_AVTS_ACTION_SET_PLAY_MODE,     NULL },
  { SERVICE_AVTS_ACTION_SET_RECORD_MODE,   NULL },
  { SERVICE_AVTS_ACTION_GET_ACTIONS,       NULL },
  { NULL,                                  NULL }
};
