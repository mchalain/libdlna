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

#define AVTS_DESCRIPTION_ACTION_SET_URI_ARGS \
"        <argument>" \
"          <name>InstanceID</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>CurrentURI</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>AVTransportURI</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>CurrentURIMetaData</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>AVTransportURIMetaData</relatedStateVariable>" \
"        </argument>"

#define AVTS_DESCRIPTION_ACTION_SET_URI \
"    <action>" \
"      <name>SetAVTransportURI</name>" \
"      <argumentList>" \
AVTS_DESCRIPTION_ACTION_SET_URI_ARGS \
"      </argumentList>" \
"    </action>"

#define AVTS_DESCRIPTION_ACTION_SET_NEXT_URI_ARGS \
"        <argument>" \
"          <name>InstanceID</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>NextURI</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>NextAVTransportURI</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>NextURIMetaData</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>NextAVTransportURIMetaData</relatedStateVariable>" \
"        </argument>"

#define AVTS_DESCRIPTION_ACTION_SET_NEXT_URI \
"    <action>" \
"      <name>SetNextAVTransportURI</name>" \
"      <argumentList>" \
AVTS_DESCRIPTION_ACTION_SET_NEXT_URI_ARGS \
"      </argumentList>" \
"    </action>"

#define AVTS_DESCRIPTION_ACTION_STOP_ARGS \
"        <argument>" \
"          <name>InstanceID</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
"        </argument>"

#define AVTS_DESCRIPTION_ACTION_STOP \
"    <action>" \
"      <name>Stop</name>" \
"      <argumentList>" \
AVTS_DESCRIPTION_ACTION_STOP_ARGS \
"      </argumentList>" \
"    </action>"

#define AVTS_DESCRIPTION_ACTION_PLAY_ARGS \
"        <argument>" \
"          <name>InstanceID</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>Speed</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>TansportPlaySpeed</relatedStateVariable>" \
"        </argument>"

#define AVTS_DESCRIPTION_ACTION_PLAY \
"    <action>" \
"      <name>Play</name>" \
"      <argumentList>" \
AVTS_DESCRIPTION_ACTION_PLAY_ARGS \
"      </argumentList>" \
"    </action>"

#define AVTS_DESCRIPTION_ACTION_PAUSE_ARGS \
"        <argument>" \
"          <name>InstanceID</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
"        </argument>"

#define AVTS_DESCRIPTION_ACTION_PAUSE \
"    <action>" \
"      <name>Pause</name>" \
"      <argumentList>" \
AVTS_DESCRIPTION_ACTION_PAUSE_ARGS \
"      </argumentList>" \
"    </action>"

#define AVTS_DESCRIPTION_ACTION_NEXT_ARGS \
"        <argument>" \
"          <name>InstanceID</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
"        </argument>"

#define AVTS_DESCRIPTION_ACTION_NEXT \
"    <action>" \
"      <name>Next</name>" \
"      <argumentList>" \
AVTS_DESCRIPTION_ACTION_NEXT_ARGS \
"      </argumentList>" \
"    </action>"

#define AVTS_DESCRIPTION_ACTION_PREVIOUS_ARGS \
"        <argument>" \
"          <name>InstanceID</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
"        </argument>"

#define AVTS_DESCRIPTION_ACTION_PREVIOUS \
"    <action>" \
"      <name>Previous</name>" \
"      <argumentList>" \
AVTS_DESCRIPTION_ACTION_PREVIOUS_ARGS \
"      </argumentList>" \
"    </action>"


#define AVTS_DESCRIPTION \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">" \
"  <specVersion>" \
"     <major>1</major>" \
"     <minor>0</minor>" \
"  </specVersion>" \
"  <actionList>" \
AVTS_DESCRIPTION_ACTION_SET_URI \
AVTS_DESCRIPTION_ACTION_SET_NEXT_URI \
AVTS_DESCRIPTION_ACTION_STOP \
AVTS_DESCRIPTION_ACTION_PLAY \
AVTS_DESCRIPTION_ACTION_PAUSE \
AVTS_DESCRIPTION_ACTION_NEXT \
AVTS_DESCRIPTION_ACTION_PREVIOUS \
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

#define AVTS_DESCRIPTION_LEN strlen (AVTS_DESCRIPTION)

#define AVTS_LOCATION "/services/avts.xml"

#define AVTS_SERVICE_ID   "urn:upnp-org:serviceId:ContentDirectory"
#define AVTS_SERVICE_TYPE "urn:schemas-upnp-org:service:ContentDirectory:1"

#define AVTS_URL              "avts.xml"
#define AVTS_CONTROL_URL      "avts_control"
#define AVTS_EVENT_URL        "avts_event"

#endif /* AVTS_H */
