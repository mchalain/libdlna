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
#include "avts.h"

#define AVTS_ERR_ACTION_FAILED                 501

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
ACTION_ARG_IN(AVTS_ARG_NEXT_URI,AVTS_VAR_AVT_URI) \
ACTION_ARG_IN(AVTS_ARG_NEXT_URI_METADATA,AVTS_VAR_AVT_URI_METADATA)

#define AVTS_ACTION_SET_NEXT_URI_ARGS \
AVTS_ACTION_ARG_INSTANCE_ID \
ACTION_ARG_IN(AVTS_ARG_CURRENT_URI,AVTS_VAR_NEXT_AVT_URI) \
ACTION_ARG_IN(AVTS_ARG_CURRENT_URI_METADATA,AVTS_VAR_NEXT_AVT_URI_METADATA)

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

#define AVTS_DESCRIPTION \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">" \
"  <specVersion>" \
"     <major>1</major>" \
"     <minor>0</minor>" \
"  </specVersion>" \
"  <actionList>" \
ACTION(AVTS_ACTION_GET_MEDIA_INFO, AVTS_ACTION_GET_MEDIA_INFO_ARGS) \
ACTION(AVTS_ACTION_GET_MEDIA_INFO_EXT, AVTS_ACTION_GET_MEDIA_INFO_EXT_ARGS) \
ACTION(AVTS_ACTION_GET_INFO, AVTS_ACTION_GET_INFO_ARGS) \
ACTION(AVTS_ACTION_GET_POS_INFO, AVTS_ACTION_GET_POS_INFO_ARGS) \
ACTION(AVTS_ACTION_GET_CAPS, AVTS_ACTION_GET_CAPS_ARGS) \
ACTION(AVTS_ACTION_GET_SETTINGS, AVTS_ACTION_GET_SETTINGS_ARGS) \
ACTION(AVTS_ACTION_SET_URI, AVTS_ACTION_SET_URI_ARGS) \
ACTION(AVTS_ACTION_SET_NEXT_URI,AVTS_ACTION_SET_NEXT_URI_ARGS) \
ACTION(AVTS_ACTION_STOP,AVTS_ACTION_ARG_INSTANCE_ID) \
ACTION(AVTS_ACTION_PLAY,AVTS_ACTION_PLAY_ARGS) \
ACTION(AVTS_ACTION_PAUSE,AVTS_ACTION_ARG_INSTANCE_ID) \
ACTION(AVTS_ACTION_SEEK,AVTS_ACTION_SEEK_ARGS) \
ACTION(AVTS_ACTION_NEXT,AVTS_ACTION_ARG_INSTANCE_ID) \
ACTION(AVTS_ACTION_PREVIOUS,AVTS_ACTION_ARG_INSTANCE_ID) \
"  </actionList>" \
"  <serviceStateTable>" \
STATEVARIABLE(AVTS_VAR_STATE,STRING,"no") \
STATEVARIABLE(AVTS_VAR_STATUS,STRING,"no") \
STATEVARIABLE(AVTS_VAR_MEDIA_CATEGORY,STRING,"no") \
STATEVARIABLE(AVTS_VAR_PLAY_MEDIUM,STRING,"no") \
STATEVARIABLE(AVTS_VAR_REC_MEDIUM,STRING,"no") \
STATEVARIABLE(AVTS_VAR_POSSIBLE_PLAY_MEDIA,STRING,"no") \
STATEVARIABLE(AVTS_VAR_POSSIBLE_REC_MEDIA,STRING,"no") \
STATEVARIABLE(AVTS_VAR_PLAY_MODE,STRING,"no") \
STATEVARIABLE(AVTS_VAR_PLAY_SPEED,STRING,"no") \
STATEVARIABLE(AVTS_VAR_REC_WRITE_STATUS,STRING,"no") \
STATEVARIABLE(AVTS_VAR_REC_QUALITY,STRING,"no") \
STATEVARIABLE(AVTS_VAR_POSSIBLE_REC_QUALITY_MODES,STRING,"no") \
STATEVARIABLE(AVTS_VAR_NB_OF_TRACKS,UI4,"no") \
STATEVARIABLE(AVTS_VAR_TRACK,UI4,"no") \
STATEVARIABLE(AVTS_VAR_TRACK_DURATION,STRING,"no") \
STATEVARIABLE(AVTS_VAR_MEDIA_DURATION,STRING,"no") \
STATEVARIABLE(AVTS_VAR_TRACK_METADATA,STRING,"no") \
STATEVARIABLE(AVTS_VAR_TRACK_URI,STRING,"no") \
STATEVARIABLE(AVTS_VAR_AVT_URI,STRING,"no") \
STATEVARIABLE(AVTS_VAR_AVT_URI_METADATA,STRING,"no") \
STATEVARIABLE(AVTS_VAR_NEXT_AVT_URI,STRING,"no") \
STATEVARIABLE(AVTS_VAR_NEXT_AVT_URI_METADATA,STRING,"no") \
STATEVARIABLE(AVTS_VAR_RTIME,STRING,"no") \
STATEVARIABLE(AVTS_VAR_ATIME,STRING,"no") \
STATEVARIABLE(AVTS_VAR_RCOUNT,I4,"no") \
STATEVARIABLE(AVTS_VAR_ACOUNT,UI4,"no") \
STATEVARIABLE("CurrentTransportActions",STRING,"no") \
STATEVARIABLE("LastChange",STRING,"no") \
STATEVARIABLE("DRMState",STRING,"no") \
STATEVARIABLE("SyncOffset",STRING,"no") \
STATEVARIABLE(AVTS_VAR_A_ARG_TYPE_SEEK_MODE,STRING,"no") \
STATEVARIABLE(AVTS_VAR_A_ARG_TYPE_SEEK_TARGET,STRING,"no") \
STATEVARIABLE(AVTS_VAR_A_ARG_TYPE_INSTANCE_ID,UI4,"no") \
"  </serviceStateTable>" \
"</scpd>"

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
  { AVTS_ACTION_SET_REC_MODE,      NULL },
  { AVTS_ACTION_GET_ACTIONS,       NULL },
  { NULL,                         NULL }
};

static char *
avts_get_description (dlna_t *dlna)
{
  return strdup(AVTS_DESCRIPTION);
}

upnp_service_t avts_service = {
  .id           = AVTS_SERVICE_ID,
  .type         = AVTS_SERVICE_TYPE,
  .scpd_url     = AVTS_URL,
  .control_url  = AVTS_CONTROL_URL,
  .event_url    = AVTS_EVENT_URL,
  .actions      = avts_service_actions,
  .get_description     = avts_get_description,
};
