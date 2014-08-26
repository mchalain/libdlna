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

#ifndef RCS_H
#define RCS_H

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
"    </action>" \

#define RCS_VAR_LAST_CHANGE "LastChange"
#define RCS_VAR_PRESET_NAME_LIST "PresetNameList"
#define RCS_VAR_BRIGHTNESS "Brightness"
#define RCS_VAR_CONTRAST "Contrast"
#define RCS_VAR_SHARPNESS "Sharpness"

#define RCS_DESCRIPTION_VAR_RED_VIDEO_GAIN \
"    <stateVariable sendEvents=\"no\">" \
"      <name>RedVideoGain</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_GREEN_VIDEO_GAIN \
"    <stateVariable sendEvents=\"no\">" \
"      <name>GreenVideoGain</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_BLUE_VIDEO_GAIN \
"    <stateVariable sendEvents=\"no\">" \
"      <name>BlueVideoGain</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_RED_VIDEO_BLACK_LEVEL \
"    <stateVariable sendEvents=\"no\">" \
"      <name>RedVideoBlackLevel</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_GREEN_VIDEO_BLACK_LEVEL \
"    <stateVariable sendEvents=\"no\">" \
"      <name>GreenVideoBlackLevel</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_BLUE_VIDEO_BLACK_LEVEL \
"    <stateVariable sendEvents=\"no\">" \
"      <name>BlueVideoBlackLevel</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_COLOR_TEMPERATURE \
"    <stateVariable sendEvents=\"no\">" \
"      <name>ColorTemperature</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_HORIZONTAL_KEYSTONE \
"    <stateVariable sendEvents=\"no\">" \
"      <name>HorizontalKeystone</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_VERTICAL_KEYSTONE \
"    <stateVariable sendEvents=\"no\">" \
"      <name>VerticalKeystone</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_MUTE \
"    <stateVariable sendEvents=\"no\">" \
"      <name>Mute</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_VOLUME \
"    <stateVariable sendEvents=\"no\">" \
"      <name>Volume</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_VOLUME_DB \
"    <stateVariable sendEvents=\"no\">" \
"      <name>VolumeDB</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_LOUDNESS \
"    <stateVariable sendEvents=\"no\">" \
"      <name>Loudness</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_ALLOWED_TRANSFORM_SETTINGS \
"    <stateVariable sendEvents=\"no\">" \
"      <name>AllowedTransformSettings</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_TRANSFORM_SETTINGS \
"    <stateVariable sendEvents=\"no\">" \
"      <name>TransformSettings</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_ALLOWED_DEFAULT_TRANSFORM_SETTINGS \
"    <stateVariable sendEvents=\"no\">" \
"      <name>AllowedDefaultTransformSettings</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_DESCRIPTION_VAR_DEFAULT_TRANSFORM_SETTINGS \
"    <stateVariable sendEvents=\"no\">" \
"      <name>DefaultTransformSettings</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>"

#define RCS_VAR_A_ARG_TYPE_CHANNEL "A_ARG_TYPE_Channel"
#define RCS_VAR_A_ARG_TYPE_INSTANCE_ID "A_ARG_TYPE_InstanceID"
#define RCS_VAR_A_ARG_TYPE_DEVICE_UDN "A_ARG_TYPE_DeviceUDN"
#define RCS_VAR_A_ARG_TYPE_SERVICE_TYPE "A_ARG_TYPE_ServiceType"
#define RCS_VAR_A_ARG_TYPE_SERVICE_ID "A_ARG_TYPE_ServiceID"
#define RCS_VAR_A_ARG_TYPE_STATE_VARIABLE_VALUE_PAIRS "A_ARG_TYPE_StateVariableValuePairs"
#define RCS_VAR_A_ARG_TYPE_STATE_VARIABLE_LIST "A_ARG_TYPE_StateVariableList"

/* RCS Action Names */
#define RCS_ACTION_LIST_PRESETS       "ListPresets"
#define RCS_ACTION_SELECT_PRESET      "SelectPreset"
#define RCS_ACTION_GET_BRIGHTNESS     "GetBrightness"
#define RCS_ACTION_SET_BRIGHTNESS     "SetBrightness"
#define RCS_ACTION_GET_CONTRAST       "GetContrast"
#define RCS_ACTION_SET_CONTRAST       "SetContrast"
#define RCS_ACTION_GET_SHARPNESS      "GetSharpness"
#define RCS_ACTION_SET_SHARPNESS      "SetSharpness"
#define RCS_ACTION_GET_R_V_GAIN       "GetRedVideoGain"
#define RCS_ACTION_SET_R_V_GAIN       "SetRedVideoGain"
#define RCS_ACTION_GET_G_V_GAIN       "GetGreenVideoGain"
#define RCS_ACTION_SET_G_V_GAIN       "SetGreenVideoGain"
#define RCS_ACTION_GET_B_V_GAIN       "GetBlueVideoGain"
#define RCS_ACTION_SET_B_V_GAIN       "SetBlueVideoGain"
#define RCS_ACTION_GET_R_V_BLEVEL     "GetRedVideoBlackLevel"
#define RCS_ACTION_SET_R_V_BLEVEL     "SetRedVideoBlackLevel"
#define RCS_ACTION_GET_G_V_BLEVEL     "GetGreenVideoBlackLevel"
#define RCS_ACTION_SET_G_V_BLEVEL     "SetGreenVideoBlackLevel"
#define RCS_ACTION_GET_B_V_BLEVEL     "GetBlueVideoBlackLevel"
#define RCS_ACTION_SET_B_V_BLEVEL     "SetBlueVideoBlackLevel"

#define RCS_DESCRIPTION \
"<?xml version=\"1.0\" encoding=RCS_VAR_A_ARG_TYPE_SERVICE_TYPE\"utf-8\"?>" \
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">" \
"  <specVersion>" \
"     <major>1</major>" \
"     <minor>0</minor>" \
"  </specVersion>" \
"  <actionList>" \
ACTION(RCS_ACTION_LIST_PRESETS,"") \
ACTION(RCS_ACTION_SELECT_PRESET,"") \
ACTION(RCS_ACTION_GET_BRIGHTNESS,"") \
ACTION(RCS_ACTION_SET_BRIGHTNESS,"") \
"  </actionList>" \
"  <serviceStateTable>" \
STATEVARIABLE(RCS_VAR_LAST_CHANGE,UI4,"no") \
STATEVARIABLE(RCS_VAR_PRESET_NAME_LIST,UI4,"no") \
STATEVARIABLE(RCS_VAR_BRIGHTNESS,UI4,"no") \
STATEVARIABLE(RCS_VAR_CONTRAST,UI4,"no") \
STATEVARIABLE(RCS_VAR_SHARPNESS,UI4,"no") \
\
STATEVARIABLE(RCS_VAR_A_ARG_TYPE_CHANNEL,UI4,"no") \
STATEVARIABLE(RCS_VAR_A_ARG_TYPE_INSTANCE_ID,UI4,"no") \
STATEVARIABLE(RCS_VAR_A_ARG_TYPE_DEVICE_UDN,STRING,"no") \
STATEVARIABLE(RCS_VAR_A_ARG_TYPE_SERVICE_TYPE,STRING,"no") \
STATEVARIABLE(RCS_VAR_A_ARG_TYPE_SERVICE_ID,STRING,"no") \
STATEVARIABLE(RCS_VAR_A_ARG_TYPE_STATE_VARIABLE_VALUE_PAIRS,STRING,"no") \
STATEVARIABLE(RCS_VAR_A_ARG_TYPE_STATE_VARIABLE_LIST,STRING,"no") \
"  </serviceStateTable>" \
"</scpd>"

#define RCS_DESCRIPTION_LEN strlen (RCS_DESCRIPTION)

#define RCS_SERVICE           "RenderingControl"
#define RCS_SERVICE_VERSION   "1"
#define RCS_SERVICE_ID   "urn:upnp-org:serviceId:"RCS_SERVICE
#define RCS_SERVICE_TYPE "urn:schemas-upnp-org:service:"RCS_SERVICE":"RCS_SERVICE_VERSION

#define RCS_URL              "rcs.xml"
#define RCS_CONTROL_URL      "rcs_control"
#define RCS_EVENT_URL        "rcss_event"

#define RCS_LOCATION "/services/"RCS_URL

#endif /* AVTS_H */
