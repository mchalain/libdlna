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

#include "upnp_internals.h"
#include "avts.h"

#define AVTS_DESCRIPTION \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">" \
"  <specVersion>" \
"     <major>1</major>" \
"     <minor>0</minor>" \
"  </specVersion>" \
"  <actionList>" \
"  </actionList>" \
"  <serviceStateTable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>TransportState</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>TransportStatus</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>PlaybackStorageMedium</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>RecordStorageMedium</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>PossiblePlaybackStorageMedia</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">"\
"     <name>PossibleRecordStorageMedia</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>CurrentPlayMode</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>TransportPlaySpeed</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>RecordMediumWriteStatus</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>CurrentRecordQualityMode</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>PossibleRecordQualityModes</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>NumberOfTracks</name>" \
"      <dataType>ui4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>CurrentTrack</name>" \
"      <dataType>ui4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>CurrentTrackDuration</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>CurrentMediaDuration</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>CurrentTrackMetaData</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>CurrentTrackURI</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>AVTransportURI</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>AVTransportURIMetaData</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>NextAVTransportURI</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>NextAVTransportURIMetaData</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>RelativeTimePosition</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>AbsoluteTimePosition</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>RelativeCounterPosition</name>" \
"      <dataType>i4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>AbsoluteCounterPosition</name>" \
"      <dataType>i4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>CurrentTransportActions</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>LastChange</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_SeekMode</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_SeekTarget</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_InstanceID</name>" \
"      <dataType>ui4</dataType>" \
"    </stateVariable>" \
"  </serviceStateTable>" \
"</scpd>"

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
    
/* List of UPnP AVTransport Service actions */
upnp_service_action_t avts_service_actions[] = {
  { SERVICE_AVTS_ACTION_SET_URI,           NULL },
  { SERVICE_AVTS_ACTION_SET_NEXT_URI,      NULL },
  { SERVICE_AVTS_ACTION_GET_MEDIA_INFO,    NULL },
  { SERVICE_AVTS_ACTION_GET_INFO,          NULL },
  { SERVICE_AVTS_ACTION_GET_POS_INFO,      NULL },
  { SERVICE_AVTS_ACTION_GET_CAPS,          NULL },
  { SERVICE_AVTS_ACTION_GET_SETTINGS,      NULL },
  { SERVICE_AVTS_ACTION_STOP,              NULL },
  { SERVICE_AVTS_ACTION_PLAY,              NULL },
  { SERVICE_AVTS_ACTION_PAUSE,             NULL },
  { SERVICE_AVTS_ACTION_RECORD,            NULL },
  { SERVICE_AVTS_ACTION_SEEK,              NULL },
  { SERVICE_AVTS_ACTION_NEXT,              NULL },
  { SERVICE_AVTS_ACTION_PREVIOUS,          NULL },
  { SERVICE_AVTS_ACTION_SET_PLAY_MODE,     NULL },
  { SERVICE_AVTS_ACTION_SET_RECORD_MODE,   NULL },
  { SERVICE_AVTS_ACTION_GET_ACTIONS,       NULL },
  { NULL,                                  NULL }
};

static char *
avts_get_description (dlna_t *dlna)
{
  return strdup(AVTS_DESCRIPTION);
}

upnp_service_t avts_service = {
  .id           = AVTS_SERVICE_ID,
  .location     = AVTS_LOCATION,
  .type         = AVTS_SERVICE_TYPE,
  .scpd_url     = AVTS_URL,
  .control_url  = AVTS_CONTROL_URL,
  .event_url    = AVTS_EVENT_URL,
  .actions      = avts_service_actions,
  .get_description     = avts_get_description,
};
