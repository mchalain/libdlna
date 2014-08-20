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

static int
avts_set_uri (dlna_t *dlna, upnp_action_event_t *ev)
{
  char *URI, *URIMetadata;
  dlna_dmp_item_t *item;
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

  item = malloc (sizeof(dlna_dmp_item_t));
  memset (item, 0, sizeof(dlna_dmp_item_t));
  item->item = dlna_item_new (dlna, URI);
  /* set id with the id of the last item + 1 */
  item->id = crc32(0, URI, strlen(URI));
  HASH_ADD_INT (dlna->dmp.playlist, id, item);

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
  dlna_dmp_item_t *item;
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

  item = malloc (sizeof(dlna_dmp_item_t));
  memset (item, 0, sizeof(dlna_dmp_item_t));
  item->item = dlna_item_new (dlna, URI);
  /* set id with the id of the last item + 1 */
  item->id = (uint32_t)(dlna->dmp.playlist->hh.tbl->tail->key) + 1;
  HASH_ADD_INT (dlna->dmp.playlist, id, item);

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

  //dmr_set_uri(dlna, URI, URIMetadata);
  
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

  //dmr_set_uri(dlna, URI, URIMetadata);
  
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

  //dmr_set_uri(dlna, URI, URIMetadata);
  
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

  //dmr_set_uri(dlna, URI, URIMetadata);
  
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
