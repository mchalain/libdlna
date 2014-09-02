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
#include "services.h"
#include "didl.h"
#include "avts.h"

#define AVTS_ERR_ACTION_FAILED                 501
#define AVTS_ERR_INVALID_INSTANCE              718 

#define AVTS_VAR_STATE                      "TransportState"
#define AVTS_VAR_STATUS                     "TransportStatus"
#define AVTS_VAR_NB_OF_TRACKS               "NumberOfTracks"
#define AVTS_VAR_A_ARG_TYPE_INSTANCE_ID     "A_ARG_TYPE_InstanceID"
#define AVTS_VAR_A_ARG_TYPE_SEEK_MODE       "A_ARG_TYPE_SeekMode"
#define AVTS_VAR_A_ARG_TYPE_SEEK_TARGET     "A_ARG_TYPE_SeekTarget"
#define AVTS_VAR_AVT_URI                    "AVTransportURI"
#define AVTS_VAR_AVT_URI_METADATA           "AVTransportURIMetaData"
#define AVTS_VAR_NEXT_AVT_URI               "NextAVTransportURI"
#define AVTS_VAR_NEXT_AVT_URI_METADATA      "NextAVTransportURIMetaData"
#define AVTS_VAR_TRANSPORT_PLAY_SPEED       "TransportPlaySpeed"
#define AVTS_VAR_MEDIA_DURATION             "CurrentMediaDuration"
#define AVTS_VAR_PLAY_MEDIUM                "PlaybackStorageMedium"
#define AVTS_VAR_POSSIBLE_PLAY_MEDIA        "PossiblePlaybackStorageMedia"
#define AVTS_VAR_REC_MEDIUM                 "RecordStorageMedium"
#define AVTS_VAR_POSSIBLE_REC_MEDIA         "PossibleRecordStorageMedia"
#define AVTS_VAR_POSSIBLE_REC_QUALITY_MODES "PossibleRecordQualityModes"
#define AVTS_VAR_REC_WRITE_STATUS           "RecordMediumWriteStatus"
#define AVTS_VAR_MEDIA_CATEGORY             "CurrentMediaCategory"
#define AVTS_VAR_PLAY_SPEED                 "TransportPlaySpeed"
#define AVTS_VAR_TRACK                      "CurrentTrack"
#define AVTS_VAR_TRACK_DURATION             "CurrentTrackDuration"
#define AVTS_VAR_TRACK_METADATA             "CurrentTrackMetaData"
#define AVTS_VAR_TRACK_URI                  "CurrentTrackURI"
#define AVTS_VAR_RTIME                      "RelativeTimePosition"
#define AVTS_VAR_ATIME                      "AbsoluteTimePosition"
#define AVTS_VAR_RCOUNT                     "RelativeCounterPosition"
#define AVTS_VAR_ACOUNT                     "AbsoluteCounterPosition"
#define AVTS_VAR_PLAY_MODE                  "CurrentPlayMode"
#define AVTS_VAR_REC_QUALITY                "CurrentRecordQualityMode"

#define AVTS_ARG_INSTANCEID            "InstanceID"
#define AVTS_ARG_INSTANCEID            "InstanceID"
#define AVTS_ARG_CURRENT_URI           "CurrentURI"
#define AVTS_ARG_NEXT_URI              "NextURI"
#define AVTS_ARG_CURRENT_URI_METADATA  "CurrentURIMetaData"
#define AVTS_ARG_NEXT_URI_METADATA     "NextURIMetaData"
#define AVTS_ARG_SPEED                 "Speed"
#define AVTS_ARG_CURRENT_SPEED         "CurrentSpeed"
#define AVTS_ARG_MEDIA_DURATION        "MediaDuration"
#define AVTS_ARG_PLAY_MEDIUM           "PlayMedium"
#define AVTS_ARG_PLAY_MEDIA            "PlayMedia"
#define AVTS_ARG_REC_MEDIUM            "RecordMedium"
#define AVTS_ARG_REC_MEDIA             "RecMedia"
#define AVTS_ARG_REC_QUALITY_MODES     "RecQualityModes"
#define AVTS_ARG_WRITE_STATUS          "WriteStatus"
#define AVTS_ARG_CURRENT_TYPE          "CurrentType"
#define AVTS_ARG_NR_TRACKS             "NrTracks"
#define AVTS_ARG_STATE                 "CurrentTransportState"
#define AVTS_ARG_STATUS                "CurrentTransportStatus"
#define AVTS_ARG_TRACK                 "Track"
#define AVTS_ARG_TRACK_DURATION        "TrackDuration"
#define AVTS_ARG_TRACK_METADATA        "TrackMetaData"
#define AVTS_ARG_TRACK_URI             "TrackURI"
#define AVTS_ARG_RTIME                 "RelTime"
#define AVTS_ARG_ATIME                 "AbsTime"
#define AVTS_ARG_RCOUNT                "RelCount"
#define AVTS_ARG_ACOUNT                "AbsCount"
#define AVTS_ARG_PLAY_MODE             "PlayMode"
#define AVTS_ARG_REC_QUALITY           "RecQualityMode"
#define AVTS_ARG_SEEK_UNIT             "Unit"
#define AVTS_ARG_SEEK_TARGET           "Target"

#define AVTS_ACTION_ARG_INSTANCE_ID ACTION_ARG_IN(AVTS_ARG_INSTANCEID,AVTS_VAR_A_ARG_TYPE_INSTANCE_ID)

/* AVTS Action Names */
#define AVTS_ACTION_SET_URI            "SetAVTransportURI"
#define AVTS_ACTION_SET_NEXT_URI       "SetNextAVTransportURI"
#define AVTS_ACTION_GET_MEDIA_INFO     "GetMediaInfo"
#define AVTS_ACTION_GET_MEDIA_INFO_EXT "GetMediaInfo_Ext"
#define AVTS_ACTION_GET_INFO           "GetTransportInfo"
#define AVTS_ACTION_GET_POS_INFO       "GetPositionInfo"
#define AVTS_ACTION_GET_CAPS           "GetDeviceCapabilities"
#define AVTS_ACTION_GET_SETTINGS       "GetTransportSettings"
#define AVTS_ACTION_STOP               "Stop"
#define AVTS_ACTION_PLAY               "Play"
#define AVTS_ACTION_PAUSE              "Pause"
#define AVTS_ACTION_RECORD             "Record"
#define AVTS_ACTION_SEEK               "Seek"
#define AVTS_ACTION_NEXT               "Next"
#define AVTS_ACTION_PREVIOUS           "Previous"
#define AVTS_ACTION_SET_PLAY_MODE      "SetPlayMode"
#define AVTS_ACTION_SET_REC_MODE       "SetRecordQualityMode"
#define AVTS_ACTION_GET_ACTIONS        "GetCurrentTransportActions"

#define AVTS_ACTION_SET_URI_ARGS \
AVTS_ACTION_ARG_INSTANCE_ID \
ACTION_ARG_IN(AVTS_ARG_CURRENT_URI,AVTS_VAR_AVT_URI) \
ACTION_ARG_IN(AVTS_ARG_CURRENT_URI_METADATA,AVTS_VAR_AVT_URI_METADATA)

#define AVTS_ACTION_SET_NEXT_URI_ARGS \
AVTS_ACTION_ARG_INSTANCE_ID \
ACTION_ARG_IN(AVTS_ARG_NEXT_URI,AVTS_VAR_NEXT_AVT_URI) \
ACTION_ARG_IN(AVTS_ARG_NEXT_URI_METADATA,AVTS_VAR_NEXT_AVT_URI_METADATA)

#define AVTS_ACTION_PLAY_ARGS \
AVTS_ACTION_ARG_INSTANCE_ID \
ACTION_ARG_IN(AVTS_ARG_SPEED,AVTS_VAR_TRANSPORT_PLAY_SPEED)

#define AVTS_ACTION_GET_MEDIA_INFO_ARGS \
AVTS_ACTION_ARG_INSTANCE_ID \
ACTION_ARG_OUT(AVTS_ARG_NR_TRACKS,AVTS_VAR_NB_OF_TRACKS) \
ACTION_ARG_OUT(AVTS_ARG_MEDIA_DURATION,AVTS_VAR_MEDIA_DURATION) \
ACTION_ARG_OUT(AVTS_ARG_CURRENT_URI,AVTS_VAR_AVT_URI) \
ACTION_ARG_OUT(AVTS_ARG_CURRENT_URI_METADATA,AVTS_VAR_AVT_URI_METADATA) \
ACTION_ARG_OUT(AVTS_ARG_NEXT_URI,AVTS_VAR_NEXT_AVT_URI) \
ACTION_ARG_OUT(AVTS_ARG_NEXT_URI_METADATA,AVTS_VAR_NEXT_AVT_URI_METADATA) \
ACTION_ARG_OUT(AVTS_ARG_PLAY_MEDIUM,AVTS_VAR_PLAY_MEDIUM) \
ACTION_ARG_OUT(AVTS_ARG_REC_MEDIUM,AVTS_VAR_REC_MEDIUM) \
ACTION_ARG_OUT(AVTS_ARG_WRITE_STATUS,AVTS_VAR_REC_WRITE_STATUS)

#define AVTS_ACTION_GET_MEDIA_INFO_EXT_ARGS \
AVTS_ACTION_ARG_INSTANCE_ID \
ACTION_ARG_OUT(AVTS_ARG_CURRENT_TYPE,AVTS_VAR_MEDIA_CATEGORY) \
ACTION_ARG_OUT(AVTS_ARG_NR_TRACKS,AVTS_VAR_NB_OF_TRACKS) \
ACTION_ARG_OUT(AVTS_ARG_MEDIA_DURATION,AVTS_VAR_MEDIA_DURATION) \
ACTION_ARG_OUT(AVTS_ARG_CURRENT_URI,AVTS_VAR_AVT_URI) \
ACTION_ARG_OUT(AVTS_ARG_CURRENT_URI_METADATA,AVTS_VAR_AVT_URI_METADATA) \
ACTION_ARG_OUT(AVTS_ARG_NEXT_URI,AVTS_VAR_NEXT_AVT_URI) \
ACTION_ARG_OUT(AVTS_ARG_NEXT_URI_METADATA,AVTS_VAR_NEXT_AVT_URI_METADATA) \
ACTION_ARG_OUT(AVTS_ARG_PLAY_MEDIUM,AVTS_VAR_PLAY_MEDIUM) \
ACTION_ARG_OUT(AVTS_ARG_REC_MEDIUM,AVTS_VAR_REC_MEDIUM) \
ACTION_ARG_OUT(AVTS_ARG_WRITE_STATUS,AVTS_VAR_REC_WRITE_STATUS)

#define AVTS_ACTION_GET_INFO_ARGS \
AVTS_ACTION_ARG_INSTANCE_ID \
ACTION_ARG_OUT(AVTS_ARG_STATE,AVTS_VAR_STATE) \
ACTION_ARG_OUT(AVTS_ARG_STATUS,AVTS_VAR_STATUS) \
ACTION_ARG_OUT(AVTS_ARG_CURRENT_SPEED,AVTS_VAR_PLAY_SPEED)

#define AVTS_ACTION_GET_POS_INFO_ARGS \
AVTS_ACTION_ARG_INSTANCE_ID \
ACTION_ARG_OUT(AVTS_ARG_TRACK,AVTS_VAR_TRACK) \
ACTION_ARG_OUT(AVTS_ARG_TRACK_DURATION,AVTS_VAR_TRACK_DURATION) \
ACTION_ARG_OUT(AVTS_ARG_TRACK_METADATA,AVTS_VAR_TRACK_METADATA) \
ACTION_ARG_OUT(AVTS_ARG_TRACK_URI,AVTS_VAR_TRACK_URI) \
ACTION_ARG_OUT(AVTS_ARG_RTIME,AVTS_VAR_RTIME) \
ACTION_ARG_OUT(AVTS_ARG_ATIME,AVTS_VAR_ATIME) \
ACTION_ARG_OUT(AVTS_ARG_RCOUNT,AVTS_VAR_RCOUNT) \
ACTION_ARG_OUT(AVTS_ARG_ACOUNT,AVTS_VAR_ACOUNT)

#define AVTS_ACTION_GET_CAPS_ARGS \
AVTS_ACTION_ARG_INSTANCE_ID \
ACTION_ARG_OUT(AVTS_ARG_PLAY_MEDIA,AVTS_VAR_POSSIBLE_PLAY_MEDIA) \
ACTION_ARG_OUT(AVTS_ARG_REC_MEDIA,AVTS_VAR_POSSIBLE_REC_MEDIA) \
ACTION_ARG_OUT(AVTS_ARG_REC_QUALITY_MODES,AVTS_VAR_POSSIBLE_REC_QUALITY_MODES)

#define AVTS_ACTION_GET_SETTINGS_ARGS \
AVTS_ACTION_ARG_INSTANCE_ID \
ACTION_ARG_OUT(AVTS_ARG_PLAY_MODE,AVTS_VAR_PLAY_MODE) \
ACTION_ARG_OUT(AVTS_ARG_REC_QUALITY,AVTS_VAR_REC_QUALITY)

#define AVTS_ACTION_SEEK_ARGS \
AVTS_ACTION_ARG_INSTANCE_ID \
ACTION_ARG_OUT(AVTS_ARG_SEEK_UNIT,AVTS_VAR_PLAY_MODE) \
ACTION_ARG_OUT(AVTS_ARG_SEEK_TARGET,AVTS_VAR_REC_QUALITY)

extern uint32_t
crc32(uint32_t crc, const void *buf, size_t size);

/* DLNA Media Player Properties */
typedef struct avts_instance_s avts_instance_t;
struct avts_instance_s
{
  uint32_t id;
  dlna_dmp_item_t *playlist;
  dlna_dmp_item_t *current_item;
  enum {
    E_NO_MEDIA,
    E_STOPPED,
    E_PLAYING,
    E_PAUSING,
    E_RECORDING,
    E_TRANSITIONING,
  } state;
  ithread_mutex_t state_mutex;
  ithread_cond_t state_change;
  ithread_t playthread;
  UT_hash_handle hh;
};

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
  if (!item->item)
  {
    free (item);
  }
  else
  {
    /* set id with the id of the last item + 1 */
    item->id = crc32(0, uri, strlen(uri));
    HASH_ADD_INT (playlist, id, item);
  }

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
      return item->hh.next;
  }
  return playlist;
}

static int
playlist_count (dlna_dmp_item_t *playlist)
{
  int i = 0;
  while ((playlist = playlist->hh.next)) i++;
  return i;
}

static int
playlist_index (dlna_dmp_item_t *playlist, dlna_dmp_item_t *item)
{
  int i = 0;
  while (playlist && playlist != item)
  {
    playlist = playlist->hh.next;
    i++;
  }
  if (playlist)
    return i;
  return 0;
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

static int
instance_change_state (avts_instance_t *instance, int newstate)
{
  ithread_mutex_lock (&instance->state_mutex);
  if (instance->state != E_NO_MEDIA && newstate != -1)
  {
    switch (newstate)
    {
    case E_RECORDING:
      /* recording is not allowed */
    break;
    case E_PLAYING:
      if (instance->state != E_RECORDING)
      {
        instance->state = newstate;
        ithread_cond_signal (&instance->state_change);
      }
    break;
    case E_STOPPED:
      instance->state = newstate;
      ithread_cond_signal (&instance->state_change);
    break;
    case E_TRANSITIONING:
    case E_PAUSING:
      if (instance->state == E_PLAYING)
      {
        instance->state = newstate;
        ithread_cond_signal (&instance->state_change);
      }
    break;
    }
  }
  ithread_mutex_unlock (&instance->state_mutex);
  return instance->state;
}

static int
instance_change_current_item (avts_instance_t *instance, dlna_dmp_item_t *newitem)
{
  instance->current_item = newitem;
  return 0;
}

static void *
avts_thread_play (void *arg)
{
  avts_instance_t *instance = (avts_instance_t *) arg;
  dlna_dmp_item_t *next_item;

  instance_change_current_item(instance, playlist_next (instance->playlist, 0));
  if (!instance->current_item)
    return NULL;
  playitem_prepare (instance->current_item->item);
  while (1)
  {
    int play_frame = 0;
    int state;

    ithread_mutex_lock (&instance->state_mutex);
    state = instance->state;
    ithread_mutex_unlock (&instance->state_mutex);
    switch (state)
    {
    case E_STOPPED:
      return NULL;
    case E_PLAYING:
      play_frame = 1;
      break;
    case E_TRANSITIONING:
      next_item = playlist_next (instance->playlist, instance->current_item->id);
      if (next_item)
      {
        playitem_prepare (next_item->item);
      }
      play_frame = 1;
      break;
    case E_PAUSING:
      ithread_mutex_lock (&instance->state_mutex);
      ithread_cond_wait (&instance->state_change, &instance->state_mutex);
      ithread_mutex_unlock (&instance->state_mutex);
      break;
    }
    if (play_frame)
    {
      int ret = playitem_decodeframe (instance->current_item->item);
      if (ret == 0 && state == E_PLAYING)
      {
        instance_change_state (instance, E_TRANSITIONING);
      }
      else if (ret < 0 && state == E_PLAYING)
      {
        next_item = playlist_next (instance->playlist, instance->current_item->id);
        if (!next_item)
          instance_change_state (instance, E_STOPPED);
        else
          playitem_prepare (next_item->item);            
        instance_change_current_item(instance, next_item);
        next_item = NULL;
      }
      /* in transition, two cases:
       *   - play_item returns -1 to complete the transition and switch to the next track
       *   - play_item returns 1 to continue but a transition is requested from user
       **/
      else if (ret != 0 && state == E_TRANSITIONING)
      {
        if (!next_item)
          instance_change_state (instance, E_STOPPED);
        else
          instance_change_state (instance, E_PLAYING);
        instance_change_current_item(instance, next_item);
        next_item = NULL;
      }
    }
  }
  return NULL;
}

static avts_instance_t *
avts_set_thread (dlna_service_t *service, uint32_t id)
{
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)service->cookie;

  instance = calloc (1, sizeof(avts_instance_t));

  ithread_mutex_init (&instance->state_mutex, NULL);
  ithread_cond_init (&instance->state_change, NULL);
  instance->state = E_PAUSING;
  instance->id = id;
  HASH_ADD_INT (instances, id, instance);
  service->cookie = instances;
  ithread_create (&instance->playthread, NULL, avts_thread_play, instance);
  return instance;
}

static int
avts_set_uri (dlna_t *dlna, upnp_action_event_t *ev)
{
  char *uri, *uri_metadata;
  uint32_t instanceID;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)ev->service->cookie;

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
  HASH_FIND_INT (instances, &instanceID, instance);
  if (!instance)
  {
    instance = avts_set_thread (ev->service, instanceID);
  }
  if (instance->state == E_STOPPED)
  {
    instance->playlist = playlist_empty (instance->playlist);
  }
  
  uri   = upnp_get_string (ev->ar, AVTS_ARG_CURRENT_URI);
  uri_metadata = upnp_get_string (ev->ar, AVTS_ARG_CURRENT_URI_METADATA);
  instance->playlist = playlist_add_item (instance->playlist, dlna, uri, uri_metadata);

  free (uri);
  free (uri_metadata);

  return ev->status;
}

static int
avts_set_next_uri (dlna_t *dlna, upnp_action_event_t *ev)
{
  char *uri, *uri_metadata;
  uint32_t instanceID;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)ev->service->cookie;

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

  HASH_FIND_INT (instances, &instanceID, instance);
  if (!instance)
  {
    ev->ar->ErrCode = AVTS_ERR_INVALID_INSTANCE;
    return 0;
  }
  playlist_add_item (instance->playlist, dlna, uri, uri_metadata);

  free (uri);
  free (uri_metadata);

  return ev->status;
}

static int
avts_get_minfo (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t instanceID;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)ev->service->cookie;
  buffer_t *out;

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
  HASH_FIND_INT (instances, &instanceID, instance);
  if (!instance)
  {
    ev->ar->ErrCode = AVTS_ERR_INVALID_INSTANCE;
    return 0;
  }

  out = buffer_new ();
  buffer_appendf (out, "%u", playlist_count(instance->playlist));
  upnp_add_response (ev, AVTS_ARG_NR_TRACKS, out->buf);
  buffer_free (out);

  upnp_add_response (ev, AVTS_ARG_MEDIA_DURATION, "1");

  out = buffer_new ();
  if (instance->current_item)
    buffer_appendf (out, "%s", instance->current_item->item->filename);
  upnp_add_response (ev, AVTS_ARG_CURRENT_URI, out->buf);
  buffer_free (out);

  out = buffer_new ();
  if (instance->current_item)
    didl_add_short_item (out, instance->current_item);
  upnp_add_response (ev, AVTS_ARG_CURRENT_URI_METADATA, out->buf);
  buffer_free (out);

  out = buffer_new ();
  if (instance->current_item->hh.next)
  {
    dlna_dmp_item_t *item = instance->current_item->hh.next;
    buffer_appendf (out, "%s", item->item->filename);
  }
  upnp_add_response (ev, AVTS_ARG_NEXT_URI, out->buf);
  buffer_free (out);

  out = buffer_new ();
  if (instance->current_item->hh.next)
  {
    dlna_dmp_item_t *item = instance->current_item->hh.next;
    didl_add_short_item (out, item);
  }
  upnp_add_response (ev, AVTS_ARG_NEXT_URI_METADATA, out->buf);
  buffer_free (out);

  upnp_add_response (ev, AVTS_ARG_PLAY_MEDIUM, "NETWORK");
  upnp_add_response (ev, AVTS_ARG_REC_MEDIUM, "NOT_IMPLEMENTED");
  upnp_add_response (ev, AVTS_ARG_WRITE_STATUS, "NOT_IMPLEMENTED");

  return ev->status;
}

static int
avts_get_minfo_ext (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t instanceID;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)ev->service->cookie;
  buffer_t *out;

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
  HASH_FIND_INT (instances, &instanceID, instance);
  if (!instance)
  {
    ev->ar->ErrCode = AVTS_ERR_INVALID_INSTANCE;
    return 0;
  }

  upnp_add_response (ev, AVTS_ARG_CURRENT_TYPE, "TRACK_AWARE");

  out = buffer_new ();
  buffer_appendf (out, "%u", playlist_count(instance->playlist));
  upnp_add_response (ev, AVTS_ARG_NR_TRACKS, out->buf);
  buffer_free (out);

  upnp_add_response (ev, AVTS_ARG_MEDIA_DURATION, "1");

  out = buffer_new ();
  if (instance->current_item)
    buffer_appendf (out, "%s", instance->current_item->item->filename);
  upnp_add_response (ev, AVTS_ARG_CURRENT_URI, out->buf);
  buffer_free (out);

  out = buffer_new ();
  if (instance->current_item)
    didl_add_short_item (out, instance->current_item);
  upnp_add_response (ev, AVTS_ARG_CURRENT_URI_METADATA, out->buf);
  buffer_free (out);

  out = buffer_new ();
  if (instance->current_item->hh.next)
  {
    dlna_dmp_item_t *item = instance->current_item->hh.next;
    buffer_appendf (out, "%s", item->item->filename);
  }
  upnp_add_response (ev, AVTS_ARG_NEXT_URI, out->buf);
  buffer_free (out);

  out = buffer_new ();
  if (instance->current_item->hh.next)
  {
    dlna_dmp_item_t *item = instance->current_item->hh.next;
    didl_add_short_item (out, item);
  }
  upnp_add_response (ev, AVTS_ARG_NEXT_URI_METADATA, out->buf);
  buffer_free (out);

  upnp_add_response (ev, AVTS_ARG_PLAY_MEDIUM, "NETWORK");
  upnp_add_response (ev, AVTS_ARG_REC_MEDIUM, "NOT_IMPLEMENTED");
  upnp_add_response (ev, AVTS_ARG_WRITE_STATUS, "NOT_IMPLEMENTED");


  return ev->status;
}

static int
avts_get_info (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t instanceID;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)ev->service->cookie;

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
  HASH_FIND_INT (instances, &instanceID, instance);
  if (!instance)
  {
    ev->ar->ErrCode = AVTS_ERR_INVALID_INSTANCE;
    return 0;
  }

  switch (instance_change_state(instance, -1))
  {
  case E_NO_MEDIA:
    upnp_add_response (ev, AVTS_ARG_STATE, "NO_MEDIA_PRESENT");
    break;
  case E_STOPPED:
    upnp_add_response (ev, AVTS_ARG_STATE, "STOPPED");
    break;
  case E_PLAYING:
    upnp_add_response (ev, AVTS_ARG_STATE, "PLAYING");
    break;
  case E_TRANSITIONING:
    upnp_add_response (ev, AVTS_ARG_STATE, "TRANSITIONING");
    break;
  case E_PAUSING:
    upnp_add_response (ev, AVTS_ARG_STATE, "PAUSED_PLAYBACK");
    break;
  case E_RECORDING:
    upnp_add_response (ev, AVTS_ARG_STATE, "RECORDING");
    break;
  }
  upnp_add_response (ev, AVTS_ARG_STATUS, "OK");
  upnp_add_response (ev, AVTS_ARG_CURRENT_SPEED, "1");

  return ev->status;
}

static int
avts_get_pos_info (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t instanceID;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)ev->service->cookie;
  buffer_t *out;
  int index = 0;

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
  HASH_FIND_INT (instances, &instanceID, instance);
  if (!instance)
  {
    ev->ar->ErrCode = AVTS_ERR_INVALID_INSTANCE;
    return 0;
  }

  out = buffer_new ();
  if (instance->current_item)
    index = playlist_index (instance->playlist, instance->current_item);
  buffer_appendf (out, "%u", index);
  upnp_add_response (ev, AVTS_ARG_TRACK, out->buf);
  buffer_free (out);
  upnp_add_response (ev, AVTS_ARG_TRACK, "");

  out = buffer_new ();
  if (instance->current_item && instance->current_item->item->properties)
    buffer_appendf (out, "%s", instance->current_item->item->properties->duration);
  upnp_add_response (ev, AVTS_ARG_TRACK_DURATION, out->buf);
  buffer_free (out);

  upnp_add_response (ev, AVTS_ARG_TRACK_METADATA, "NOT_IMPLEMENTED");

  out = buffer_new ();
  if (instance->current_item)
    buffer_appendf (out, "%s", instance->current_item->item->filename);
  upnp_add_response (ev, AVTS_ARG_TRACK_URI, out->buf);
  buffer_free (out);

  upnp_add_response (ev, AVTS_ARG_RTIME, "NOT_IMPLEMENTED");
  upnp_add_response (ev, AVTS_ARG_ATIME, "NOT_IMPLEMENTED");
  upnp_add_response (ev, AVTS_ARG_RCOUNT, "NOT_IMPLEMENTED");
  upnp_add_response (ev, AVTS_ARG_ACOUNT, "NOT_IMPLEMENTED");

  return ev->status;
}

static int
avts_get_dev_caps (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t instanceID;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)ev->service->cookie;

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
  HASH_FIND_INT (instances, &instanceID, instance);
  if (!instance)
  {
    ev->ar->ErrCode = AVTS_ERR_INVALID_INSTANCE;
    return 0;
  }

  upnp_add_response (ev, AVTS_ARG_PLAY_MEDIA, "NETWORK");
  upnp_add_response (ev, AVTS_ARG_REC_MEDIA, "NOT_IMPLEMENTED");
  upnp_add_response (ev, AVTS_ARG_REC_QUALITY_MODES, "NOT_IMPLEMENTED");

  return ev->status;
}

static int
avts_get_settings (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t instanceID;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)ev->service->cookie;

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
  HASH_FIND_INT (instances, &instanceID, instance);
  if (!instance)
  {
    ev->ar->ErrCode = AVTS_ERR_INVALID_INSTANCE;
    return 0;
  }

  upnp_add_response (ev, AVTS_ARG_PLAY_MODE, "NORMAL");
  upnp_add_response (ev, AVTS_ARG_REC_QUALITY, "NOT_IMPLEMENTED");

  return ev->status;
}

static int
avts_play (dlna_t *dlna, upnp_action_event_t *ev)
{
  int speed;
  uint32_t instanceID;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)ev->service->cookie;

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
  HASH_FIND_INT (instances, &instanceID, instance);
  if (!instance)
  {
    ev->ar->ErrCode = AVTS_ERR_INVALID_INSTANCE;
    return 0;
  }
  speed = upnp_get_ui4 (ev->ar, AVTS_ARG_SPEED);

  instance_change_state(instance, E_PLAYING);

  return ev->status;
}

static int
avts_stop (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t instanceID;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)ev->service->cookie;

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
  HASH_FIND_INT ((avts_instance_t *)ev->service->cookie, &instanceID, instance);
  if (!instance)
  {
    ev->ar->ErrCode = AVTS_ERR_INVALID_INSTANCE;
    return 0;
  }

  instance_change_state(instance, E_STOPPED);
  ithread_join (instance->playthread, NULL);
  HASH_DEL (instances, instance);

  return ev->status;
}

static int
avts_pause (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t instanceID;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)ev->service->cookie;

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
  HASH_FIND_INT (instances, &instanceID, instance);
  if (!instance)
  {
    ev->ar->ErrCode = AVTS_ERR_INVALID_INSTANCE;
    return 0;
  }

  instance_change_state(instance, E_PAUSING);

  return ev->status;
}

static int
avts_next (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t instanceID;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)ev->service->cookie;

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
  HASH_FIND_INT (instances, &instanceID, instance);
  if (!instance)
  {
    ev->ar->ErrCode = AVTS_ERR_INVALID_INSTANCE;
    return 0;
  }
  
  instance_change_state (instance, E_TRANSITIONING);

  return ev->status;
}

static int
avts_previous (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t instanceID;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)ev->service->cookie;

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
  HASH_FIND_INT (instances, &instanceID, instance);
  if (!instance)
  {
    ev->ar->ErrCode = AVTS_ERR_INVALID_INSTANCE;
    return 0;
  }

  return ev->status;
}

/* List of UPnP AVTransport Service actions */
upnp_service_action_t avts_service_actions[] = {
  { AVTS_ACTION_SET_URI, AVTS_ACTION_SET_URI_ARGS,           avts_set_uri },
  { AVTS_ACTION_SET_NEXT_URI, AVTS_ACTION_SET_NEXT_URI_ARGS,      avts_set_next_uri },
  { AVTS_ACTION_GET_MEDIA_INFO, AVTS_ACTION_GET_MEDIA_INFO_ARGS,    avts_get_minfo },
  { AVTS_ACTION_GET_MEDIA_INFO_EXT, AVTS_ACTION_GET_MEDIA_INFO_EXT_ARGS,    avts_get_minfo_ext },
  { AVTS_ACTION_GET_INFO, AVTS_ACTION_GET_INFO_ARGS,          avts_get_info },
  { AVTS_ACTION_GET_POS_INFO, AVTS_ACTION_GET_POS_INFO_ARGS,      avts_get_pos_info },
  { AVTS_ACTION_GET_CAPS, AVTS_ACTION_GET_CAPS_ARGS,          avts_get_dev_caps },
  { AVTS_ACTION_GET_SETTINGS, AVTS_ACTION_GET_SETTINGS_ARGS,      avts_get_settings },
  { AVTS_ACTION_STOP, AVTS_ACTION_ARG_INSTANCE_ID,              avts_stop },
  { AVTS_ACTION_PLAY, AVTS_ACTION_PLAY_ARGS,              avts_play },
  { AVTS_ACTION_PAUSE, AVTS_ACTION_ARG_INSTANCE_ID,             avts_pause },
  { AVTS_ACTION_RECORD, NULL,            NULL },
  { AVTS_ACTION_SEEK, AVTS_ACTION_SEEK_ARGS,              NULL },
  { AVTS_ACTION_NEXT, AVTS_ACTION_ARG_INSTANCE_ID,              avts_next },
  { AVTS_ACTION_PREVIOUS, AVTS_ACTION_ARG_INSTANCE_ID,          avts_previous },
  { AVTS_ACTION_SET_PLAY_MODE, NULL,     NULL },
  { AVTS_ACTION_SET_REC_MODE, NULL,     NULL },
  { AVTS_ACTION_GET_ACTIONS, NULL,       NULL },
  { NULL, NULL,                        NULL }
};

upnp_service_statevar_t avts_service_variables[] = {
  {AVTS_VAR_STATE,E_STRING,0, NULL},
  {AVTS_VAR_STATUS,E_STRING,0, NULL},
  {AVTS_VAR_MEDIA_CATEGORY,E_STRING,0, NULL},
  {AVTS_VAR_PLAY_MEDIUM,E_STRING,0, NULL},
  {AVTS_VAR_REC_MEDIUM,E_STRING,0, NULL},
  {AVTS_VAR_POSSIBLE_PLAY_MEDIA,E_STRING,0, NULL},
  {AVTS_VAR_POSSIBLE_REC_MEDIA,E_STRING,0, NULL},
  {AVTS_VAR_PLAY_MODE,E_STRING,0, NULL},
  {AVTS_VAR_PLAY_SPEED,E_STRING,0, NULL},
  {AVTS_VAR_REC_WRITE_STATUS,E_STRING,0, NULL},
  {AVTS_VAR_REC_QUALITY,E_STRING,0, NULL},
  {AVTS_VAR_POSSIBLE_REC_QUALITY_MODES,E_STRING,0, NULL},
  {AVTS_VAR_NB_OF_TRACKS,E_UI4,0, NULL},
  {AVTS_VAR_TRACK,E_UI4,0, NULL},
  {AVTS_VAR_TRACK_DURATION,E_STRING,0, NULL},
  {AVTS_VAR_MEDIA_DURATION,E_STRING,0, NULL},
  {AVTS_VAR_TRACK_METADATA,E_STRING,0, NULL},
  {AVTS_VAR_TRACK_URI,E_STRING,0, NULL},
  {AVTS_VAR_AVT_URI,E_STRING,0, NULL},
  {AVTS_VAR_AVT_URI_METADATA,E_STRING,0, NULL},
  {AVTS_VAR_NEXT_AVT_URI,E_STRING,0, NULL},
  {AVTS_VAR_NEXT_AVT_URI_METADATA,E_STRING,0, NULL},
  {AVTS_VAR_RTIME,E_STRING,0, NULL},
  {AVTS_VAR_ATIME,E_STRING,0, NULL},
  {AVTS_VAR_RCOUNT,E_I4,0, NULL},
  {AVTS_VAR_ACOUNT,E_UI4,0, NULL},
  {"CurrentTransportActions",E_STRING,0, NULL},
  {"LastChange",E_STRING,0, NULL},
  {"DRMState",E_STRING,0, NULL},
  {"SyncOffset",E_STRING,0, NULL},
  {AVTS_VAR_A_ARG_TYPE_SEEK_MODE,E_STRING,0, NULL},
  {AVTS_VAR_A_ARG_TYPE_SEEK_TARGET,E_STRING,0, NULL},
  {AVTS_VAR_A_ARG_TYPE_INSTANCE_ID,E_UI4,0, NULL},
  { NULL, 0, 0, NULL},
};

static char *
avts_get_description (dlna_t *dlna)
{
  return dlna_service_get_description (dlna, avts_service_actions, avts_service_variables);
}

dlna_service_t *
avts_service_new (dlna_t *dlna dlna_unused)
{
  dlna_service_t *service = NULL;
  service = calloc (1, sizeof (dlna_service_t));
  
  service->id           = AVTS_SERVICE_ID;
  service->type         = AVTS_SERVICE_TYPE;
  service->scpd_url     = AVTS_URL;
  service->control_url  = AVTS_CONTROL_URL;
  service->event_url    = AVTS_EVENT_URL;
  service->actions      = avts_service_actions;
  service->statevar     = avts_service_variables;
  service->get_description     = avts_get_description;
  service->init         = NULL;

  return service;
};
