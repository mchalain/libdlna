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

/*
 * AVTransport service specifications can be found at:
 * http://upnp.org/standardizeddcps/documents/AVTransport1.0.pdf
 * http://www.upnp.org/specs/av/UPnP-av-AVTransport-v2-Service-20060531.pdf
 */

#include <stdlib.h>
#include <stdio.h>

#include "upnp_internals.h"
#include "services.h"
#include "didl.h"
#include "avts.h"

#define AVTS_ERR_ACTION_FAILED                 501
#define AVTS_ERR_TRANSITION_NOT_AVAILABLE      701
#define AVTS_ERR_NO_CONTENTS                   702
#define AVTS_ERR_NOT_IMPLEMENTED               710
#define AVTS_ERR_ILLEGAL_MIME                  714
#define AVTS_ERR_SPEED_NOT_SUPPORTED           717
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
#define AVTS_VAR_ACTIONS                    "CurrentTransportActions"

#define AVTS_VAR_STATUS_VAL                 "OK"
#define AVTS_VAR_POSSIBLE_PLAY_MEDIA_VAL    "UNKNOWN,NETWORK"
#define AVTS_VAR_RECORD_VAL                 "NOT_IMPLEMENTED"
#define AVTS_VAR_PLAY_SPEED_VAL             "1"
#define AVTS_VAR_PLAY_MODE_VAL              "NORMAL"
#define AVTS_VAR_TRACK_DURATION_VAL_ZERO    "00:00:00"
#define AVTS_VAR_AVT_URI_VAL_EMPTY          "no track uri"

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
#define AVTS_ARG_ACTIONS               "Actions"

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
ACTION_ARG_IN(AVTS_ARG_SEEK_UNIT,AVTS_VAR_A_ARG_TYPE_SEEK_MODE) \
ACTION_ARG_IN(AVTS_ARG_SEEK_TARGET,AVTS_VAR_A_ARG_TYPE_SEEK_TARGET)

#define AVTS_ACTION_GET_ACTIONS_ARGS \
AVTS_ACTION_ARG_INSTANCE_ID \
ACTION_ARG_OUT(AVTS_ARG_ACTIONS,AVTS_VAR_ACTIONS)

#define LAST_CHANGE_ID 27 /* be careful this value depend of statevar structure */

extern uint32_t
crc32(uint32_t crc, const void *buf, size_t size);

/* DLNA Media Player Properties */
typedef struct avts_instance_s avts_instance_t;
struct avts_instance_s
{
  uint32_t id;
  avts_playlist_t *playlist;
  dlna_service_t *service;
  enum {
	E_SHUTDOWN = -1,
    E_NO_MEDIA = 0,
    E_STOPPED,
    E_PLAYING,
    E_PAUSING,
    E_RECORDING,
    E_TRANSITIONING,
  } state;
  ithread_mutex_t state_mutex;
  ithread_cond_t state_change;
  ithread_t playthread;
  uint32_t counter;
  UT_hash_handle hh;
};

char *g_TransportState[] = 
{
  "NO_MEDIA_PRESENT",
  "STOPPED",
  "PLAYING",
  "PAUSED_PLAYBACK",
  "RECORDING",
  "TRANSITIONING",
};

static avts_playlist_t *
playlist_empty (avts_playlist_t *playlist)
{
  avts_playlist_t *item;

  for (item = playlist; item; item = item->hh.next)
  {
    HASH_DEL (playlist, item);
    free (item);
  }
  return NULL;
}

static avts_playlist_t *
playlist_add_item (avts_playlist_t *playlist, dlna_t *dlna, char *uri, char *uri_metadata dlna_unused)
{
  avts_playlist_t *item = NULL;
  uint32_t id;

  /* set id with the id of the last item + 1 */
  id = crc32(0, uri, strlen(uri));
  HASH_FIND_INT (playlist, &id, item);
  if (item)
    return playlist;

  item = calloc (1, sizeof(avts_playlist_t));

  item->item = dlna_item_new (dlna, uri);
  if (!item->item)
  {
    free (item);
  }
  else
  {
    /* this item will be the first */
    if (!playlist)
      item->current = item; /* set the first item as the start of the playlist */
    HASH_ADD_INT (playlist, id, item);
  }

  return playlist;
}

static avts_playlist_t *
playlist_current (avts_playlist_t *playlist)
{
  if (!playlist)
    return NULL;
  return playlist->current;
}

static avts_playlist_t *
playlist_next (avts_playlist_t *playlist)
{
  if (!playlist)
    return NULL;
  if (playlist->current)
    return playlist->current->hh.next;
  return playlist;
}

static avts_playlist_t *
playlist_previous (avts_playlist_t *playlist)
{
  if (!playlist)
    return NULL;
  if (playlist->current)
    return playlist->current->hh.prev;
  return playlist;
}

static int
playlist_count (avts_playlist_t *playlist)
{
  int i = 0;
  if (!playlist)
    return 0;
  while ((playlist = playlist->hh.next)) i++;
  return i + 1;
}

static int
playlist_index (avts_playlist_t *playlist, avts_playlist_t *item)
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

static avts_playlist_t *
playlist_seek (avts_playlist_t *playlist, int32_t target)
{
  avts_playlist_t *item;
  if  (!playlist)
    return NULL;
  item = playlist->current;
  if (target < 0)
  {
    while (target < 0 && ((item = item->hh.prev) != NULL)) target++;
  }
  else
  {
    while (target > 0 && ((item = item->hh.next) != NULL)) target--;
  }
  return item;
}

static int
playitem_prepare (dlna_item_t *item)
{
  if (item->profile->prepare_stream)
  {
    if (item->stream)
      return -1;
    item->stream = stream_open (item->filename);
    item->profile->prepare_stream (item);
  }
  return 0;
}

static int
playitem_decodeframe (dlna_item_t *item)
{
  int ret;
  if (item->profile->read_stream)
    ret = item->profile->read_stream (item);
  return ret;
}

static int
playitem_close (dlna_item_t *item)
{
  if (item->profile->close_stream)
  {
    item->profile->close_stream (item);
    stream_close (item->stream);
    item->stream = NULL;
  }
  return 0;
}

static char *
instance_possible_state (avts_instance_t *instance)
{
  char *val = NULL;
  buffer_t *out;
  out = buffer_new();
  ithread_mutex_lock (&instance->state_mutex);
  switch (instance->state)
  {
  case E_NO_MEDIA:
  case E_RECORDING:
    buffer_appendf (out,"NONE");
  break;
  case E_TRANSITIONING:
  case E_PLAYING:
    buffer_appendf (out,"STOP,PAUSE,SEEK");
    if (playlist_next (instance->playlist))
      buffer_appendf (out,",NEXT");
    if (playlist_previous (instance->playlist))
      buffer_appendf (out,",PREVIOUS");
  break;
  case E_STOPPED:
    buffer_appendf (out,"PLAY");
  break;
  case E_PAUSING:
    buffer_appendf (out,"STOP,PLAY");
  break;
  }
  ithread_mutex_unlock (&instance->state_mutex);
  val =strdup (out->buf);
  buffer_free (out);
  return val;
}

static int
instance_change_state (avts_instance_t *instance, int newstate)
{
  int changed = 0;

  ithread_mutex_lock (&instance->state_mutex);
  if (instance->state != E_NO_MEDIA && newstate != -1)
  {
    switch (newstate)
    {
    case E_NO_MEDIA:
      instance->state = newstate;
      changed = 1;
    break;
    case E_RECORDING:
      /* recording is not allowed */
    break;
    case E_PLAYING:
      if (instance->state == E_PLAYING)
        break;
      if (instance->state != E_RECORDING)
      {
        instance->state = newstate;
        ithread_cond_signal (&instance->state_change);
        changed = 1;
      }
    break;
    case E_STOPPED:
      if (instance->state == E_STOPPED)
        break;
      instance->state = newstate;
      ithread_cond_signal (&instance->state_change);
      changed = 1;
    break;
    case E_TRANSITIONING:
      if (instance->state == E_TRANSITIONING)
        break;
    case E_PAUSING:
      if (instance->state == E_PAUSING)
        break;
      if (instance->state == E_PLAYING || instance->state == E_TRANSITIONING)
      {
        instance->state = newstate;
        ithread_cond_signal (&instance->state_change);
        changed = 1;
      }
    break;
    }
  }
  ithread_mutex_unlock (&instance->state_mutex);
  return changed;
}

int
instance_get_state (avts_instance_t *instance)
{
  int state;
  ithread_mutex_lock (&instance->state_mutex);
  state = instance->state;
  ithread_mutex_unlock (&instance->state_mutex);
  return state;
}

void
avts_request_event (dlna_service_t *service)
{
  if (service->statevar[LAST_CHANGE_ID].eventing)
    service->statevar[LAST_CHANGE_ID].eventing++;
  if (!service->statevar[LAST_CHANGE_ID].eventing)
  {
    service->statevar[LAST_CHANGE_ID].eventing = 1;
  }
}

static void *
avts_thread_play (void *arg)
{
  avts_instance_t *instance = (avts_instance_t *) arg;
  int run = 1;

  while (run)
  {
    int play_frame = 0;
    int state;

    ithread_mutex_lock (&instance->state_mutex);
    state = instance->state;
    ithread_mutex_unlock (&instance->state_mutex);
    switch (state)
    {
    case E_SHUTDOWN:
      run = 0;
      break;
    case E_PLAYING:
      play_frame = 1;
      break;
    case E_TRANSITIONING:
      play_frame = 1;
      break;
    case E_NO_MEDIA:
      ithread_mutex_lock (&instance->state_mutex);
      while (!instance->playlist && instance->state == E_NO_MEDIA)
        ithread_cond_wait (&instance->state_change, &instance->state_mutex);
      ithread_mutex_unlock (&instance->state_mutex);
      if (instance->playlist)
        playitem_prepare (instance->playlist->item);
      instance->counter = 0;
      break;
    case E_STOPPED:
      if (!instance->playlist)
      {
        if (instance_change_state (instance, E_NO_MEDIA))
          avts_request_event (instance->service);
        break;
      }
      else if (instance->playlist->current)
        playitem_close (instance->playlist->current->item);
      ithread_mutex_lock (&instance->state_mutex);
      while (instance->state == E_STOPPED)
        ithread_cond_wait (&instance->state_change, &instance->state_mutex);
      ithread_mutex_unlock (&instance->state_mutex);
      instance->playlist->current = instance->playlist;
      instance->playlist->next = playlist_next (instance->playlist);
      playitem_prepare (instance->playlist->current->item);
      instance->counter = 0;
      break;
    case E_PAUSING:
      ithread_mutex_lock (&instance->state_mutex);
      while (instance->state == E_PAUSING)
        ithread_cond_wait (&instance->state_change, &instance->state_mutex);
      ithread_mutex_unlock (&instance->state_mutex);
      break;
    }
    if (play_frame)
    {
      int ret = playitem_decodeframe (playlist_current(instance->playlist)->item);
      instance->counter++;
      if (ret > 0)
      {
        if (state == E_TRANSITIONING)
        {
      /* in transition, two cases:
       *   - play_item returns -1 to complete the transition and switch to the next track
       *   - play_item returns 1 to continue but a transition is requested from user
       **/
          if (!instance->playlist->next)
            instance_change_state (instance, E_STOPPED);
          else
          {
            instance->playlist->current = instance->playlist->next;
            instance->playlist->next = playlist_next (instance->playlist);
            instance_change_state (instance, E_PLAYING);
          }
          avts_request_event (instance->service);
        }
      }
      else if (ret == 0)
      {
        if (state == E_PLAYING)
        {
          instance_change_state (instance, E_TRANSITIONING);
          avts_request_event (instance->service);
        }
      }
      else
      {
        if (state == E_PLAYING)
        {
          if (instance->playlist->next)
          {
            playitem_close (instance->playlist->current->item);
            instance->playlist->current = instance->playlist->next;
            playitem_prepare (instance->playlist->current->item);
            instance->counter = 0;
            instance->playlist->next = playlist_next (instance->playlist);
          }
          else
          {
            instance_change_state (instance, E_STOPPED);
            avts_request_event (instance->service);
          }
        }
        if (state == E_TRANSITIONING)
        {
      /* in transition, two cases:
       *   - play_item returns -1 to complete the transition and switch to the next track
       *   - play_item returns 1 to continue but a transition is requested from user
       **/
          if (!instance->playlist->next)
            instance_change_state (instance, E_STOPPED);
          else
          {
            instance->playlist->current = instance->playlist->next;
            instance->playlist->next = playlist_next (instance->playlist);
            instance_change_state (instance, E_PLAYING);
          }
          avts_request_event (instance->service);
        }
      }
    }
  }
  return NULL;
}

static avts_instance_t *
avts_create_instance (dlna_service_t *service, uint32_t id)
{
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)service->cookie;

  instance = calloc (1, sizeof(avts_instance_t));

  ithread_mutex_init (&instance->state_mutex, NULL);
  ithread_cond_init (&instance->state_change, NULL);
  instance_change_state  (instance, E_NO_MEDIA);
  instance->playlist = NULL;
  instance->id = id;
  instance->service = service;
  HASH_ADD_INT (instances, id, instance);
  service->cookie = instances;
  ithread_create (&instance->playthread, NULL, avts_thread_play, instance);
  return instance;
}

static void
avts_kill_instance (dlna_service_t *service, uint32_t instanceID)
{
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)service->cookie;

  HASH_FIND_INT (instances, &instanceID, instance);

  if (instance)
  {
    ithread_mutex_lock (&instance->state_mutex);
    instance->state = E_SHUTDOWN;
    ithread_cond_signal (&instance->state_change);
    ithread_mutex_unlock (&instance->state_mutex);
    ithread_join (instance->playthread, NULL);
    ithread_mutex_destroy (&instance->state_mutex);
    ithread_cond_destroy (&instance->state_change);
    playlist_empty (instance->playlist);
    HASH_DEL (instances, instance);
    free (instance);
  }
  return;
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
    ev->ar->ErrCode = AVTS_ERR_INVALID_INSTANCE;
    return 0;
  }
  ithread_mutex_lock (&instance->state_mutex);
  if (instance->state == E_NO_MEDIA)
    instance->state = E_STOPPED;
  if (instance->state == E_STOPPED)
  {
    instance->playlist = playlist_empty (instance->playlist);
  }
  ithread_mutex_unlock (&instance->state_mutex);
  
  uri   = upnp_get_string (ev->ar, AVTS_ARG_CURRENT_URI);
  uri_metadata = upnp_get_string (ev->ar, AVTS_ARG_CURRENT_URI_METADATA);
  instance->playlist = playlist_add_item (instance->playlist, dlna, uri, uri_metadata);
  if (!instance->playlist)
  {
    ev->ar->ErrCode = AVTS_ERR_ILLEGAL_MIME;
    return 0;
  }
  if (instance->state == E_STOPPED)
    instance->playlist->current = instance->playlist;
  instance->playlist->next = playlist_next (instance->playlist);

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
  instance->playlist->next = playlist_next (instance->playlist);

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
  avts_playlist_t *plitem;

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

  plitem = playlist_current(instance->playlist);

  if (plitem && plitem->item->properties)
    upnp_add_response (ev, AVTS_ARG_MEDIA_DURATION, plitem->item->properties->duration);
  else
    upnp_add_response (ev, AVTS_ARG_MEDIA_DURATION, AVTS_VAR_TRACK_DURATION_VAL_ZERO);

  out = buffer_new ();
  if (plitem)
    buffer_appendf (out, "%s", plitem->item->filename);
  else
    buffer_appendf (out, "%s", AVTS_VAR_AVT_URI_VAL_EMPTY);
  upnp_add_response (ev, AVTS_ARG_CURRENT_URI, out->buf);
  buffer_free (out);

  out = buffer_new ();
  
  if (plitem)
    didl_add_item (out, plitem->id, plitem->item, 0, 1, NULL, NULL);
  upnp_add_response (ev, AVTS_ARG_CURRENT_URI_METADATA, out->buf);
  buffer_free (out);

  out = buffer_new ();
  plitem = playlist_next(instance->playlist);
  if (plitem)
  {
    buffer_appendf (out, "%s", plitem->item->filename);
  }
  upnp_add_response (ev, AVTS_ARG_NEXT_URI, out->buf);
  buffer_free (out);

  out = buffer_new ();
  if (plitem)
  {
    didl_add_item (out, plitem->id, plitem->item, 0, 1, NULL, NULL);
  }
  else
    buffer_appendf (out, "%s", AVTS_VAR_AVT_URI_VAL_EMPTY);
  upnp_add_response (ev, AVTS_ARG_NEXT_URI_METADATA, out->buf);
  buffer_free (out);

  upnp_add_response (ev, AVTS_ARG_PLAY_MEDIUM, "NETWORK");
  upnp_add_response (ev, AVTS_ARG_REC_MEDIUM, AVTS_VAR_RECORD_VAL);
  upnp_add_response (ev, AVTS_ARG_WRITE_STATUS, AVTS_VAR_RECORD_VAL);

  return ev->status;
}

static int
avts_get_minfo_ext (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t instanceID;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)ev->service->cookie;
  buffer_t *out;
  avts_playlist_t *plitem;

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

  plitem = playlist_current(instance->playlist);
  if (plitem && plitem->item->properties)
    upnp_add_response (ev, AVTS_ARG_MEDIA_DURATION, plitem->item->properties->duration);
  else
    upnp_add_response (ev, AVTS_ARG_MEDIA_DURATION, AVTS_VAR_TRACK_DURATION_VAL_ZERO);

  out = buffer_new ();
  if (plitem)
    buffer_appendf (out, "%s", plitem->item->filename);
  else
    buffer_appendf (out, "%s", AVTS_VAR_AVT_URI_VAL_EMPTY);
  upnp_add_response (ev, AVTS_ARG_CURRENT_URI, out->buf);
  buffer_free (out);

  out = buffer_new ();
  if (plitem)
    didl_add_item (out, plitem->id, plitem->item, 0, 1, NULL, NULL);
  upnp_add_response (ev, AVTS_ARG_CURRENT_URI_METADATA, out->buf);
  buffer_free (out);

  out = buffer_new ();
  plitem = playlist_next(instance->playlist);
  if (plitem)
  {
    buffer_appendf (out, "%s", plitem->item->filename);
  }
  else
    buffer_appendf (out, "%s", AVTS_VAR_AVT_URI_VAL_EMPTY);
  upnp_add_response (ev, AVTS_ARG_NEXT_URI, out->buf);
  buffer_free (out);

  out = buffer_new ();
  if (plitem)
  {
    didl_add_item (out, plitem->id, plitem->item, 0, 1, NULL, NULL);
  }
  upnp_add_response (ev, AVTS_ARG_NEXT_URI_METADATA, out->buf);
  buffer_free (out);

  upnp_add_response (ev, AVTS_ARG_PLAY_MEDIUM, "NETWORK");
  upnp_add_response (ev, AVTS_ARG_REC_MEDIUM, AVTS_VAR_RECORD_VAL);
  upnp_add_response (ev, AVTS_ARG_WRITE_STATUS, AVTS_VAR_RECORD_VAL);


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

  upnp_add_response (ev, AVTS_ARG_STATE, g_TransportState[instance->state]);
  upnp_add_response (ev, AVTS_ARG_STATUS, AVTS_VAR_STATUS_VAL);
  upnp_add_response (ev, AVTS_ARG_CURRENT_SPEED, AVTS_VAR_PLAY_SPEED_VAL);

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
  avts_playlist_t *plitem = NULL;

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

  if (instance->state != E_STOPPED)
    plitem = playlist_current(instance->playlist);
  out = buffer_new ();
  if (plitem)
    index = playlist_index (instance->playlist, plitem) + 1;
  buffer_appendf (out, "%u", index);
  upnp_add_response (ev, AVTS_ARG_TRACK, out->buf);
  buffer_free (out);

  if (plitem && plitem->item->properties)
    upnp_add_response (ev, AVTS_ARG_TRACK_DURATION, plitem->item->properties->duration);
  else
    upnp_add_response (ev, AVTS_ARG_TRACK_DURATION, AVTS_VAR_TRACK_DURATION_VAL_ZERO);

  out = buffer_new ();
  if (plitem)
    didl_add_item (out, plitem->id, plitem->item, 0, 1, NULL, NULL);
  else
    buffer_appendf (out, "%s", "");
  dlna_log (DLNA_MSG_INFO, "didl:\n %s\n", out->buf);
  upnp_add_response (ev, AVTS_ARG_TRACK_METADATA, out->buf);
  buffer_free (out);

  if (plitem)
    upnp_add_response (ev, AVTS_ARG_TRACK_URI, plitem->item->filename);
  else
    upnp_add_response (ev, AVTS_ARG_TRACK_URI, AVTS_VAR_AVT_URI_VAL_EMPTY);

  if (plitem && plitem->item->properties && plitem->item->properties->bps && plitem->item->properties->sample_frequency)
  {
    char duration[100];
    uint32_t time, time_s, time_m, time_h;
    long rate = plitem->item->properties->sample_frequency;

    time = instance->counter * plitem->item->properties->spf / rate / plitem->item->properties->bps * 8;

    time_h = time / 60 / 60;
    time_m = (time / 60) % 60;
    time_s = time % 60;
    snprintf(duration, 63, "%02u:%02u:%02u", time_h, time_m, time_s);
    upnp_add_response (ev, AVTS_ARG_RTIME, duration);
  }
  else
    upnp_add_response (ev, AVTS_ARG_RTIME, "NOT_IMPLEMENTED");
  upnp_add_response (ev, AVTS_ARG_ATIME, "NOT_IMPLEMENTED");
  out = buffer_new ();
  buffer_appendf (out, "%u", instance->counter);
  upnp_add_response (ev, AVTS_ARG_RCOUNT, out->buf);
  upnp_add_response (ev, AVTS_ARG_ACOUNT, out->buf);
  buffer_free (out);

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

  upnp_add_response (ev, AVTS_ARG_PLAY_MEDIA, AVTS_VAR_POSSIBLE_PLAY_MEDIA_VAL);
  upnp_add_response (ev, AVTS_ARG_REC_MEDIA, AVTS_VAR_RECORD_VAL);
  upnp_add_response (ev, AVTS_ARG_REC_QUALITY_MODES, AVTS_VAR_RECORD_VAL);

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

  upnp_add_response (ev, AVTS_ARG_PLAY_MODE, AVTS_VAR_PLAY_MODE_VAL);
  upnp_add_response (ev, AVTS_ARG_REC_QUALITY, AVTS_VAR_RECORD_VAL);

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
  if (speed != 1)
  {
    ev->ar->ErrCode = AVTS_ERR_SPEED_NOT_SUPPORTED;
    return 0;
  }

  if (!instance_change_state(instance, E_PLAYING))
  {
    ev->ar->ErrCode = AVTS_ERR_TRANSITION_NOT_AVAILABLE;
    return 0;
  }
  if (!instance->playlist)
  {
    ev->ar->ErrCode = AVTS_ERR_NO_CONTENTS;
    return 0;
  }
  
  avts_request_event (instance->service);

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
  HASH_FIND_INT (instances, &instanceID, instance);
  if (!instance)
  {
    ev->ar->ErrCode = AVTS_ERR_INVALID_INSTANCE;
    return 0;
  }

  if (!instance_change_state(instance, E_STOPPED))
  {
    ev->ar->ErrCode = AVTS_ERR_TRANSITION_NOT_AVAILABLE;
    return 0;
  }
  avts_request_event (instance->service);
  if (instanceID > 0)
  {
    avts_kill_instance (instance->service, instanceID);
  }

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

  if (!instance_change_state(instance, E_PAUSING))
  {
    ev->ar->ErrCode = AVTS_ERR_TRANSITION_NOT_AVAILABLE;
    return 0;
  }
  avts_request_event (instance->service);

  return ev->status;
}

static int
avts_seek (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t instanceID;
  char *unit;
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
  unit   = upnp_get_string (ev->ar, AVTS_ARG_SEEK_UNIT);
  if (!strncmp (unit, "TRACK_NR", 8))
  {
    int32_t nbtrack;
    nbtrack = upnp_get_ui4 (ev->ar, AVTS_ARG_SEEK_TARGET);
    instance->playlist->next = playlist_seek (instance->playlist, nbtrack);
  }
  else
  {
    ev->ar->ErrCode = AVTS_ERR_NOT_IMPLEMENTED;
    return 0;
  }
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
  instance->playlist->next = playlist_seek (instance->playlist, 1);
  if (!instance_change_state(instance, E_TRANSITIONING))
  {
    ev->ar->ErrCode = AVTS_ERR_TRANSITION_NOT_AVAILABLE;
    return 0;
  }
  avts_request_event (instance->service);

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

  instance->playlist->next = playlist_seek (instance->playlist, -1);
  if (!instance_change_state(instance, E_TRANSITIONING))
  {
    ev->ar->ErrCode = AVTS_ERR_TRANSITION_NOT_AVAILABLE;
    return 0;
  }
  avts_request_event (instance->service);

  return ev->status;
}

static int
avts_get_actions (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t instanceID;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)ev->service->cookie;
  char *val;

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

  val = instance_possible_state (instance);
  upnp_add_response (ev, AVTS_ARG_ACTIONS, val);
  free (val);

  return ev->status;
}

static char *
avts_get_last_change (dlna_t *dlna dlna_unused, dlna_service_t *service)
{
  char *value = NULL;
  buffer_t *out;
  avts_instance_t *instance = NULL;
  avts_instance_t *instances = (avts_instance_t *)service->cookie;

  out = buffer_new ();
  buffer_appendf (out, "<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT/\">");
  for (instance = instances; instance; instance = instance->hh.next)
  {
    int index = 0;
    char *val;
    avts_playlist_t *plitem;

    plitem = playlist_current(instance->playlist);

    buffer_appendf (out, "<InstanceID val=\"%d\">",instance->id);
    buffer_appendf (out, "<TransportState val=\"%s\"/>",g_TransportState[instance->state]);
    buffer_appendf (out, "<TransportStatus val=\"%s\"/>", AVTS_VAR_STATUS_VAL);

    val = instance_possible_state(instance);
    buffer_appendf (out, "<CurrentTransportActions val=\"%s\"/>", val);
    free (val);

    if (plitem)
    {
      buffer_appendf (out, "<AVTransportURI val=\"%s\"/>", plitem->item->filename);
      buffer_appendf (out, "<AVTransportURMetaData val=\"");
      //didl_add_item (out, plitem->id, plitem->item, 0, 1, NULL, NULL);
      buffer_appendf (out, "\"/>");
    }

    if (instance->playlist)
      buffer_appendf (out, "<NumberOfTracks val=\"%u\"/>", playlist_count(instance->playlist));
    if (instance_get_state (instance) == E_PLAYING)
    {
      index = playlist_index (instance->playlist, plitem) + 1;
      if (plitem)
      {
        buffer_appendf (out, "<CurrentTrackURI val=\"%s\"/>", plitem->item->filename);
        buffer_appendf (out, "<CurrentTrackMetaData val=\"");
        //didl_add_item (out, plitem->id, plitem->item, 0, 1, NULL, NULL);
        buffer_appendf (out, "\"/>");
      }
/*
      if (plitem && plitem->item->properties )
        buffer_appendf (out, "<CurrentTrackDuration val=\"%s\">", plitem->item->properties->duration);
      if (plitem && plitem->item->properties)
        buffer_appendf (out, "<CurrentMediaDuration val=\"%s\">", plitem->item->properties->duration);
*/
    }
    buffer_appendf (out, "<CurrentTrack val=\"%u\"/>", index);

    buffer_appendf (out, "<PlaybackStorageMedium val=\"UNKNOWN\"/>");
    buffer_appendf (out, "<RecordStorageMedium val=\"%s\"/>",AVTS_VAR_RECORD_VAL);
    buffer_appendf (out, "<CurrentPlayMode val=\"NORMAL\"/>");
    buffer_appendf (out, "<TransportPlaySpeed val=\"%s\"/>", AVTS_VAR_PLAY_SPEED_VAL);
    buffer_appendf (out, "<RecordMediumWriteStatus val=\"%s\"/>", AVTS_VAR_RECORD_VAL);
    buffer_appendf (out, "<CurrentRecordQualityMode val=\"%s\"/>", AVTS_VAR_RECORD_VAL);
    buffer_appendf (out, "<PossiblePlaybackStorageMedia val=\"%s\"/>", AVTS_VAR_POSSIBLE_PLAY_MEDIA_VAL);
    buffer_appendf (out, "<PossibleRecordStorageMedia val=\"%s\"/>", AVTS_VAR_RECORD_VAL);
    buffer_appendf (out, "<PossibleRecordQualityModes val=\"%s\"/>", AVTS_VAR_RECORD_VAL);
    buffer_appendf (out, "</InstanceID>");
  }
  buffer_appendf (out, "</Event>");

  value = strdup (out->buf);
  buffer_free (out);
  
  return value;
}

/* List of UPnP AVTransport Service actions */
upnp_service_action_t avts_service_actions[] = {
  { .name = AVTS_ACTION_SET_URI, 
    .args = AVTS_ACTION_SET_URI_ARGS, 
    .args_s = NULL, 
    .cb = avts_set_uri },
  { .name = AVTS_ACTION_SET_NEXT_URI, 
    .args = AVTS_ACTION_SET_NEXT_URI_ARGS, 
    .args_s = NULL,
    .cb = avts_set_next_uri },
  { .name = AVTS_ACTION_GET_MEDIA_INFO, 
    .args = AVTS_ACTION_GET_MEDIA_INFO_ARGS, 
    .args_s = NULL,
    .cb = avts_get_minfo },
  { .name = AVTS_ACTION_GET_MEDIA_INFO_EXT, 
    .args = AVTS_ACTION_GET_MEDIA_INFO_EXT_ARGS, 
    .args_s = NULL,
    .cb = avts_get_minfo_ext },
  { .name = AVTS_ACTION_GET_INFO, 
    .args = AVTS_ACTION_GET_INFO_ARGS, 
    .args_s = NULL,
    .cb = avts_get_info },
  { .name = AVTS_ACTION_GET_POS_INFO, 
    .args = AVTS_ACTION_GET_POS_INFO_ARGS,
    .cb = avts_get_pos_info },
  { .name = AVTS_ACTION_GET_CAPS, 
    .args = AVTS_ACTION_GET_CAPS_ARGS,
    .cb = avts_get_dev_caps },
  { .name = AVTS_ACTION_GET_SETTINGS, 
    .args = AVTS_ACTION_GET_SETTINGS_ARGS,
    .cb = avts_get_settings },
  { .name = AVTS_ACTION_STOP, 
    .args = AVTS_ACTION_ARG_INSTANCE_ID,
    .cb = avts_stop },
  { .name = AVTS_ACTION_PLAY, 
    .args = AVTS_ACTION_PLAY_ARGS,
    .cb = avts_play },
  { .name = AVTS_ACTION_PAUSE, 
    .args = AVTS_ACTION_ARG_INSTANCE_ID,
    .cb = avts_pause },
  { .name = AVTS_ACTION_RECORD, 
    .args = NULL,
    .cb = NULL },
  { .name = AVTS_ACTION_SEEK, 
    .args = AVTS_ACTION_SEEK_ARGS,
    .cb = avts_seek },
  { .name = AVTS_ACTION_NEXT, 
    .args = AVTS_ACTION_ARG_INSTANCE_ID,
    .cb = avts_next },
  { .name = AVTS_ACTION_PREVIOUS, 
    .args = AVTS_ACTION_ARG_INSTANCE_ID,
    .cb = avts_previous },
  { .name = AVTS_ACTION_SET_PLAY_MODE, 
    .args = NULL,
    .cb = NULL },
  { .name = AVTS_ACTION_SET_REC_MODE, 
    .args = NULL,
    .cb = NULL },
  { .name = AVTS_ACTION_GET_ACTIONS, 
    .args = AVTS_ACTION_GET_ACTIONS_ARGS,
    .cb = avts_get_actions },
  { .name = NULL, 
    .args = NULL,
    .cb = NULL }
};

char *AVTS_VAR_STATE_allowed[] =
{"STOPPED","PLAYING","TRANSITIONING","PAUSED_PLAYBACK","PAUSED_RECORDING","RECORDING","NO_MEDIA_PRESENT",NULL};
char *AVTS_VAR_PLAY_MODE_allowed[] =
{"NORMAL","SHUFFLE","REPEAT_ONE","REPEAT_ALL","RANDOM","DIRECT_1","INTRO",NULL};
char *AVTS_VAR_A_ARG_TYPE_SEEK_MODE_allowed[] =
{"TRACK_NR",NULL};


upnp_service_statevar_t avts_service_variables[] = {
  {AVTS_VAR_STATE,E_STRING,0, AVTS_VAR_STATE_allowed, NULL},
  {AVTS_VAR_STATUS,E_STRING,0, NULL, NULL},
  {AVTS_VAR_MEDIA_CATEGORY,E_STRING,0, NULL, NULL},
  {AVTS_VAR_PLAY_MEDIUM,E_STRING,0, NULL, NULL},
  {AVTS_VAR_REC_MEDIUM,E_STRING,0, NULL, NULL},
  {AVTS_VAR_POSSIBLE_PLAY_MEDIA,E_STRING,0, NULL, NULL},
  {AVTS_VAR_POSSIBLE_REC_MEDIA,E_STRING,0, NULL, NULL},
  {AVTS_VAR_PLAY_MODE,E_STRING,0, AVTS_VAR_PLAY_MODE_allowed, NULL},
  {AVTS_VAR_PLAY_SPEED,E_STRING,0, NULL, NULL},
  {AVTS_VAR_REC_WRITE_STATUS,E_STRING,0, NULL, NULL},
  {AVTS_VAR_REC_QUALITY,E_STRING,0, NULL, NULL},
  {AVTS_VAR_POSSIBLE_REC_QUALITY_MODES,E_STRING,0, NULL, NULL},
  {AVTS_VAR_NB_OF_TRACKS,E_UI4,0, NULL, NULL},
  {AVTS_VAR_TRACK,E_UI4,0, NULL, NULL},
  {AVTS_VAR_TRACK_DURATION,E_STRING,0, NULL, NULL},
  {AVTS_VAR_MEDIA_DURATION,E_STRING,0, NULL, NULL},
  {AVTS_VAR_TRACK_METADATA,E_STRING,0, NULL, NULL},
  {AVTS_VAR_TRACK_URI,E_STRING,0, NULL, NULL},
  {AVTS_VAR_AVT_URI,E_STRING,0, NULL, NULL},
  {AVTS_VAR_AVT_URI_METADATA,E_STRING,0, NULL, NULL},
  {AVTS_VAR_NEXT_AVT_URI,E_STRING,0, NULL, NULL},
  {AVTS_VAR_NEXT_AVT_URI_METADATA,E_STRING,0, NULL, NULL},
  {AVTS_VAR_RTIME,E_STRING,0, NULL, NULL},
  {AVTS_VAR_ATIME,E_STRING,0, NULL, NULL},
  {AVTS_VAR_RCOUNT,E_I4,0, NULL, NULL},
  {AVTS_VAR_ACOUNT,E_UI4,0, NULL, NULL},
  {AVTS_VAR_ACTIONS,E_STRING,0, NULL, NULL},
  {"LastChange",E_STRING,1, NULL, avts_get_last_change},
  {"DRMState",E_STRING,0, NULL, NULL},
  {"SyncOffset",E_STRING,0, NULL, NULL},
  {AVTS_VAR_A_ARG_TYPE_SEEK_MODE,E_STRING,0, AVTS_VAR_A_ARG_TYPE_SEEK_MODE_allowed, NULL},
  {AVTS_VAR_A_ARG_TYPE_SEEK_TARGET,E_STRING,0, NULL, NULL},
  {AVTS_VAR_A_ARG_TYPE_INSTANCE_ID,E_UI4,0, NULL, NULL},
  { NULL, 0, 0, NULL, NULL},
};

static char *
avts_get_description (dlna_service_t *service dlna_unused)
{
  return dlna_service_get_description (avts_service_actions, avts_service_variables);
}

static void
avts_free (dlna_service_t *service)
{
  avts_instance_t *instance;
  avts_instance_t *instances = (avts_instance_t *)service->cookie;

  for (instance = instances; instance; instance = instance->hh.next)
  {
    avts_kill_instance (service, instance->id);
  }
}

dlna_service_t *
avts_service_new (dlna_t *dlna dlna_unused)
{
  avts_instance_t *instance = NULL;
  dlna_service_t *service = NULL;
  service = calloc (1, sizeof (dlna_service_t));
  
  service->id           = AVTS_SERVICE_ID;
  service->typeid       = DLNA_SERVICE_AV_TRANSPORT;
  service->type         = AVTS_SERVICE_TYPE;
  service->scpd_url     = AVTS_URL;
  service->control_url  = AVTS_CONTROL_URL;
  service->event_url    = AVTS_EVENT_URL;
  service->actions      = avts_service_actions;
  service->statevar     = avts_service_variables;
  service->get_description     = avts_get_description;
  service->init         = NULL;
  service->free         = avts_free;
  service->last_change  = 1;

  instance = avts_create_instance (service, 0);
  service->cookie = instance;

  return service;
};
