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

#ifndef AVTS_H
#define AVTS_H

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

#define STRING "string"
#define BOOLEAN "boolean"
#define I2 "i2"
#define UI2 "ui2"
#define I4 "i4"
#define UI4 "ui4"
#define URI "uri"

#define STATEVARIABLE(name,type,eventing) \
"    <stateVariable sendEvents=\""eventing"\">" \
"      <name>"name"</name>" \
"      <dataType>"type"</dataType>" \
"    </stateVariable>"

#define ACTION(name,args) \
"    <action>" \
"      <name>"name"</name>" \
"      <argumentList>" \
args \
"      </argumentList>" \
"    </action>"
#define ACTION_ARG_IN(name,variable) \
"        <argument>" \
"          <name>"name"</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>"variable"</relatedStateVariable>" \
"        </argument>"

#define ACTION_ARG_OUT(name,variable) \
"        <argument>" \
"          <name>"name"</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>"variable"</relatedStateVariable>" \
"        </argument>"

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

#define AVTS_DESCRIPTION_LEN strlen (AVTS_DESCRIPTION)

#define AVTS_LOCATION "/services/avts.xml"

#define AVTS_SERVICE_ID   "urn:upnp-org:serviceId:ContentDirectory"
#define AVTS_SERVICE_TYPE "urn:schemas-upnp-org:service:ContentDirectory:1"

#define AVTS_URL              "avts.xml"
#define AVTS_CONTROL_URL      "avts_control"
#define AVTS_EVENT_URL        "avts_event"

#endif /* AVTS_H */
