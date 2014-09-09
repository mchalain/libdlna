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
#include "rcs.h"

#define RCS_VAR_LAST_CHANGE "LastChange"
#define RCS_VAR_PRESET_NAME_LIST "PresetNameList"
#define RCS_VAR_BRIGHTNESS "Brightness"
#define RCS_VAR_CONTRAST "Contrast"
#define RCS_VAR_SHARPNESS "Sharpness"
#define RCS_VAR_RED_VIDEO_GAIN "RedVideoGain"
#define RCS_VAR_GREEN_VIDEO_GAIN "GreenVideoGain"
#define RCS_VAR_BLUE_VIDEO_GAIN "BlueVideoGain"
#define RCS_VAR_RED_VIDEO_BLACK_LEVEL "RedVideoBlackLevel"
#define RCS_VAR_GREEN_VIDEO_BLACK_LEVEL "GreenVideoBlackLevel"
#define RCS_VAR_BLUE_VIDEO_BLACK_LEVEL "BlueVideoBlackLevel"
#define RCS_VAR_COLOR_TEMPERATURE "ColorTemperature"
#define RCS_VAR_HORIZONTAL_KEYSTONE "HorizontalKeystone"
#define RCS_VAR_VERTICAL_KEYSTONE "VerticalKeystone"
#define RCS_VAR_MUTE "Mute"
#define RCS_VAR_VOLUME "Volume"
#define RCS_VAR_VOLUME_DB "VolumeDB"
#define RCS_VAR_LOUDNESS "Loudness"
#define RCS_VAR_ALLOWED_TRANSFORM_SETTINGS "AllowedTransformSettings"
#define RCS_VAR_TRANSFORM_SETTINGS "TransformSettings"
#define RCS_VAR_ALLOWED_DEFAULT_TRANSFORM_SETTINGS "AllowedDefaultTransformSettings"
#define RCS_VAR_DEFAULT_TRANSFORM_SETTINGS "DefaultTransformSettings"
#define RCS_VAR_A_ARG_TYPE_CHANNEL "A_ARG_TYPE_Channel"
#define RCS_VAR_A_ARG_TYPE_INSTANCE_ID "A_ARG_TYPE_InstanceID"
#define RCS_VAR_A_ARG_TYPE_PRESET_NAME "A_ARG_TYPE_PresetName"
#define RCS_VAR_A_ARG_TYPE_DEVICE_UDN "A_ARG_TYPE_DeviceUDN"
#define RCS_VAR_A_ARG_TYPE_SERVICE_TYPE "A_ARG_TYPE_ServiceType"
#define RCS_VAR_A_ARG_TYPE_SERVICE_ID "A_ARG_TYPE_ServiceID"
#define RCS_VAR_A_ARG_TYPE_STATE_VARIABLE_VALUE_PAIRS "A_ARG_TYPE_StateVariableValuePairs"
#define RCS_VAR_A_ARG_TYPE_STATE_VARIABLE_LIST "A_ARG_TYPE_StateVariableList"

#define RCS_ACTION_ARG_INSTANCE_ID \
"        <argument>" \
"          <name>InstanceID</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
"        </argument>"

#define ACTION_SET_ARGS(var) \
"        <argument>" \
"          <name>Desired"var"</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>"var"</relatedStateVariable>" \
"        </argument>"
#define ACTION_GET_ARGS(var) \
"        <argument>" \
"          <name>Current"var"</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>"var"</relatedStateVariable>" \
"        </argument>"

#define ACTION_GET(var) \
ACTION("Get"var,RCS_ACTION_ARG_INSTANCE_ID ACTION_GET_ARGS(var))
#define ACTION_SET(var) \
ACTION("Set"var,RCS_ACTION_ARG_INSTANCE_ID ACTION_SET_ARGS(var))

/* RCS Action Names */
#define RCS_ACTION_LIST_PRESETS       "ListPresets"
#define RCS_ACTION_LIST_PRESETS_ARGS \
RCS_ACTION_ARG_INSTANCE_ID \
"        <argument>" \
"          <name>Current"RCS_VAR_PRESET_NAME_LIST"</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>"RCS_VAR_PRESET_NAME_LIST"</relatedStateVariable>" \
"        </argument>"
#define RCS_ACTION_SELECT_PRESET      "SelectPreset"
#define RCS_ACTION_SELECT_PRESET_ARGS \
RCS_ACTION_ARG_INSTANCE_ID \
"        <argument>" \
"          <name>PresetName</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>"RCS_VAR_A_ARG_TYPE_PRESET_NAME"</relatedStateVariable>" \
"        </argument>"
#define RCS_ACTION_GET_BRIGHTNESS     "Get"RCS_VAR_BRIGHTNESS
#define RCS_ACTION_SET_BRIGHTNESS     "Set"RCS_VAR_BRIGHTNESS
#define RCS_ACTION_GET_CONTRAST       "Get"RCS_VAR_CONTRAST
#define RCS_ACTION_SET_CONTRAST       "Set"RCS_VAR_CONTRAST
#define RCS_ACTION_GET_SHARPNESS      "Get"RCS_VAR_SHARPNESS
#define RCS_ACTION_SET_SHARPNESS      "Set"RCS_VAR_SHARPNESS
#define RCS_ACTION_GET_R_V_GAIN       "Get"RCS_VAR_RED_VIDEO_GAIN
#define RCS_ACTION_SET_R_V_GAIN       "Set"RCS_VAR_RED_VIDEO_GAIN
#define RCS_ACTION_GET_G_V_GAIN       "Get"RCS_VAR_GREEN_VIDEO_GAIN
#define RCS_ACTION_SET_G_V_GAIN       "Set"RCS_VAR_GREEN_VIDEO_GAIN
#define RCS_ACTION_GET_B_V_GAIN       "Get"RCS_VAR_BLUE_VIDEO_GAIN
#define RCS_ACTION_SET_B_V_GAIN       "Set"RCS_VAR_BLUE_VIDEO_GAIN
#define RCS_ACTION_GET_R_V_BLEVEL     "Get"RCS_VAR_RED_VIDEO_BLACK_LEVEL
#define RCS_ACTION_SET_R_V_BLEVEL     "Set"RCS_VAR_RED_VIDEO_BLACK_LEVEL
#define RCS_ACTION_GET_G_V_BLEVEL     "Get"RCS_VAR_GREEN_VIDEO_BLACK_LEVEL
#define RCS_ACTION_SET_G_V_BLEVEL     "Set"RCS_VAR_GREEN_VIDEO_BLACK_LEVEL
#define RCS_ACTION_GET_B_V_BLEVEL     "Get"RCS_VAR_BLUE_VIDEO_BLACK_LEVEL
#define RCS_ACTION_SET_B_V_BLEVEL     "Set"RCS_VAR_BLUE_VIDEO_BLACK_LEVEL
#define RCS_ACTION_GET_COLOR_TEMPERATURE     "Get"RCS_VAR_COLOR_TEMPERATURE
#define RCS_ACTION_SET_COLOR_TEMPERATURE     "Set"RCS_VAR_COLOR_TEMPERATURE
#define RCS_ACTION_GET_HORIZONTAL_KEYSTONE   "Get"RCS_VAR_HORIZONTAL_KEYSTONE
#define RCS_ACTION_SET_HORIZONTAL_KEYSTONE   "Set"RCS_VAR_HORIZONTAL_KEYSTONE
#define RCS_ACTION_GET_VERTICAL_KEYSTONE     "Get"RCS_VAR_VERTICAL_KEYSTONE
#define RCS_ACTION_SET_VERTICAL_KEYSTONE     "Set"RCS_VAR_VERTICAL_KEYSTONE
#define RCS_ACTION_GET_MUTE           "Get"RCS_VAR_MUTE
#define RCS_ACTION_SET_MUTE           "Set"RCS_VAR_MUTE
#define RCS_ACTION_GET_VOLUME         "Get"RCS_VAR_VOLUME
#define RCS_ACTION_SET_VOLUME         "Set"RCS_VAR_VOLUME
#define RCS_ACTION_GET_VOLUME_DB      "Get"RCS_VAR_VOLUME_DB
#define RCS_ACTION_SET_VOLUME_DB      "Set"RCS_VAR_VOLUME_DB
#define RCS_ACTION_GET_VOLUME_DB_RANGE       "GetVolumeDBRange"
#define RCS_ACTION_GET_VOLUME_DB_RANGE_ARGS \
RCS_ACTION_ARG_INSTANCE_ID \
"        <argument>" \
"          <name>Channel</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>"RCS_VAR_A_ARG_TYPE_CHANNEL"</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>MinValue</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>"RCS_VAR_VOLUME_DB"</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>MaxValue</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>"RCS_VAR_VOLUME_DB"</relatedStateVariable>" \
"        </argument>"

#define RCS_ACTION_GET_LOUDNESS       "Get"RCS_VAR_LOUDNESS
#define RCS_ACTION_SET_LOUDNESS       "Set"RCS_VAR_LOUDNESS
#define RCS_ACTION_GET_STATE_VARIABLES       "GetStateVariables"
#define RCS_ACTION_GET_STATE_VARIABLES_ARGS \
RCS_ACTION_ARG_INSTANCE_ID \
"        <argument>" \
"          <name>StateVariableList</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>"RCS_VAR_A_ARG_TYPE_STATE_VARIABLE_LIST"</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>StateVariableValuePairs</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>"RCS_VAR_A_ARG_TYPE_STATE_VARIABLE_VALUE_PAIRS"</relatedStateVariable>" \
"        </argument>"

#define RCS_ACTION_SET_STATE_VARIABLES       "SetStateVariables"
#define RCS_ACTION_SET_STATE_VARIABLES_ARGS \
RCS_ACTION_ARG_INSTANCE_ID \
"        <argument>" \
"          <name>RenderingControlUDN</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>"RCS_VAR_A_ARG_TYPE_DEVICE_UDN"</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>ServiceType</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>"RCS_VAR_A_ARG_TYPE_SERVICE_TYPE"</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>ServiceId</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>"RCS_VAR_A_ARG_TYPE_SERVICE_ID"</relatedStateVariable>" \
"        </argument>" \
"        </argument>" \
"        <argument>" \
"          <name>StateVariableValuePairs</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>"RCS_VAR_A_ARG_TYPE_STATE_VARIABLE_VALUE_PAIRS"</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>StateVariableList</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>"RCS_VAR_A_ARG_TYPE_STATE_VARIABLE_LIST"</relatedStateVariable>" \
"        </argument>"

#define RCS_ACTION_GET_ALLOWED_TRANSFORMS    "GetAllowedTransforms"
#define RCS_ACTION_GET_TRANSFORMS     "GetTransforms"
#define RCS_ACTION_GET_TRANSFORMS_ARGS \
RCS_ACTION_ARG_INSTANCE_ID \
"        <argument>" \
"          <name>CurrentTransformValues</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>"RCS_VAR_TRANSFORM_SETTINGS"</relatedStateVariable>" \
"        </argument>"

#define RCS_ACTION_SET_TRANSFORMS     "SetTransforms"
#define RCS_ACTION_SET_TRANSFORMS_ARGS \
RCS_ACTION_ARG_INSTANCE_ID \
"        <argument>" \
"          <name>DesiredTransformValues</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>"RCS_VAR_TRANSFORM_SETTINGS"</relatedStateVariable>" \
"        </argument>"

#define RCS_ACTION_GET_ALLOWED_DEFAULT_TRANSFORMS    "GetAllowedDefaultTransforms"
#define RCS_ACTION_GET_ALLOWED_DEFAULT_TRANSFORMS_ARGS \
"        <argument>" \
"          <name>"RCS_VAR_ALLOWED_DEFAULT_TRANSFORM_SETTINGS"</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>"RCS_VAR_ALLOWED_DEFAULT_TRANSFORM_SETTINGS"</relatedStateVariable>" \
"        </argument>"

#define RCS_ACTION_GET_DEFAULT_TRANSFORMS    "GetDefaultTransforms"
#define RCS_ACTION_GET_DEFAULT_TRANSFORMS_ARGS \
"        <argument>" \
"          <name>Current"RCS_VAR_DEFAULT_TRANSFORM_SETTINGS"</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>"RCS_VAR_DEFAULT_TRANSFORM_SETTINGS"</relatedStateVariable>" \
"        </argument>"

#define RCS_ACTION_SET_DEFAULT_TRANSFORMS    "SetDefaultTransforms"
#define RCS_ACTION_SET_DEFAULT_TRANSFORMS_ARGS \
"        <argument>" \
"          <name>Desired"RCS_VAR_DEFAULT_TRANSFORM_SETTINGS"</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>"RCS_VAR_DEFAULT_TRANSFORM_SETTINGS"</relatedStateVariable>" \
"        </argument>"

#define RCS_ACTION_GET_ALL_AVAILABLE_TRANSFORMS    "GetAllAvailableTransforms"
#define RCS_ACTION_GET_ALL_AVAILABLE_TRANSFORMS_ARGS \
"        <argument>" \
"          <name>AllAllowedTransformSettings</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>"RCS_VAR_ALLOWED_TRANSFORM_SETTINGS"</relatedStateVariable>" \
"        </argument>"

/* List of UPnP Rendering Control Service actions */
upnp_service_action_t rcs_service_actions[] = {
  { RCS_ACTION_LIST_PRESETS, RCS_ACTION_LIST_PRESETS_ARGS,          NULL },
  { RCS_ACTION_SELECT_PRESET, RCS_ACTION_SELECT_PRESET_ARGS,     NULL },
  { RCS_ACTION_GET_BRIGHTNESS, ACTION_GET_ARGS(RCS_VAR_BRIGHTNESS),   NULL },
  { RCS_ACTION_SET_BRIGHTNESS, ACTION_SET_ARGS(RCS_VAR_BRIGHTNESS),          NULL },
  { RCS_ACTION_GET_CONTRAST, ACTION_GET_ARGS(RCS_VAR_CONTRAST),      NULL },
  { RCS_ACTION_SET_CONTRAST, ACTION_SET_ARGS(RCS_VAR_CONTRAST),          NULL },
  { RCS_ACTION_GET_SHARPNESS, ACTION_GET_ARGS(RCS_VAR_SHARPNESS),      NULL },
  { RCS_ACTION_SET_SHARPNESS, ACTION_SET_ARGS(RCS_VAR_SHARPNESS),              NULL },
  { RCS_ACTION_GET_R_V_GAIN, ACTION_GET_ARGS(RCS_VAR_RED_VIDEO_GAIN),              NULL },
  { RCS_ACTION_SET_R_V_GAIN, ACTION_SET_ARGS(RCS_VAR_RED_VIDEO_GAIN),             NULL },
  { RCS_ACTION_GET_G_V_GAIN, ACTION_GET_ARGS(RCS_VAR_GREEN_VIDEO_GAIN),            NULL },
  { RCS_ACTION_SET_G_V_GAIN, ACTION_SET_ARGS(RCS_VAR_GREEN_VIDEO_GAIN),              NULL },
  { RCS_ACTION_GET_B_V_GAIN, ACTION_GET_ARGS(RCS_VAR_BLUE_VIDEO_GAIN),              NULL },
  { RCS_ACTION_SET_B_V_GAIN, ACTION_SET_ARGS(RCS_VAR_BLUE_VIDEO_GAIN),          NULL },
  { RCS_ACTION_GET_R_V_BLEVEL, ACTION_GET_ARGS(RCS_VAR_RED_VIDEO_BLACK_LEVEL),     NULL },
  { RCS_ACTION_SET_R_V_BLEVEL, ACTION_SET_ARGS(RCS_VAR_RED_VIDEO_BLACK_LEVEL),   NULL },
  { RCS_ACTION_GET_G_V_BLEVEL, ACTION_GET_ARGS(RCS_VAR_GREEN_VIDEO_BLACK_LEVEL),       NULL },
  { RCS_ACTION_SET_G_V_BLEVEL, ACTION_SET_ARGS(RCS_VAR_GREEN_VIDEO_BLACK_LEVEL),       NULL },
  { RCS_ACTION_GET_B_V_BLEVEL, ACTION_GET_ARGS(RCS_VAR_BLUE_VIDEO_BLACK_LEVEL),       NULL },
  { RCS_ACTION_SET_B_V_BLEVEL, ACTION_SET_ARGS(RCS_VAR_BLUE_VIDEO_BLACK_LEVEL),       NULL },
  { RCS_ACTION_GET_COLOR_TEMPERATURE, ACTION_GET_ARGS(RCS_VAR_COLOR_TEMPERATURE),       NULL },
  { RCS_ACTION_SET_COLOR_TEMPERATURE, ACTION_SET_ARGS(RCS_VAR_COLOR_TEMPERATURE),       NULL },
  { RCS_ACTION_GET_HORIZONTAL_KEYSTONE, ACTION_GET_ARGS(RCS_VAR_HORIZONTAL_KEYSTONE),       NULL },
  { RCS_ACTION_SET_HORIZONTAL_KEYSTONE, ACTION_SET_ARGS(RCS_VAR_HORIZONTAL_KEYSTONE),       NULL },
  { RCS_ACTION_GET_VERTICAL_KEYSTONE, ACTION_GET_ARGS(RCS_VAR_VERTICAL_KEYSTONE),       NULL },
  { RCS_ACTION_SET_VERTICAL_KEYSTONE, ACTION_SET_ARGS(RCS_VAR_VERTICAL_KEYSTONE),       NULL },
  { RCS_ACTION_GET_MUTE, ACTION_GET_ARGS(RCS_VAR_MUTE),       NULL },
  { RCS_ACTION_SET_MUTE, ACTION_SET_ARGS(RCS_VAR_MUTE),       NULL },
  { RCS_ACTION_GET_VOLUME, ACTION_GET_ARGS(RCS_VAR_VOLUME),       NULL },
  { RCS_ACTION_SET_VOLUME, ACTION_SET_ARGS(RCS_VAR_VOLUME),       NULL },
  { RCS_ACTION_GET_VOLUME_DB, ACTION_GET_ARGS(RCS_VAR_VOLUME_DB),       NULL },
  { RCS_ACTION_SET_VOLUME_DB, ACTION_SET_ARGS(RCS_VAR_VOLUME_DB),       NULL },
  { RCS_ACTION_GET_VOLUME_DB_RANGE,RCS_ACTION_GET_VOLUME_DB_RANGE_ARGS,       NULL },
  { RCS_ACTION_GET_LOUDNESS, ACTION_GET_ARGS(RCS_VAR_LOUDNESS),       NULL },
  { RCS_ACTION_SET_LOUDNESS, ACTION_SET_ARGS(RCS_VAR_LOUDNESS),       NULL },
  { RCS_ACTION_GET_STATE_VARIABLES,RCS_ACTION_GET_STATE_VARIABLES_ARGS,       NULL },
  { RCS_ACTION_SET_STATE_VARIABLES,RCS_ACTION_SET_STATE_VARIABLES_ARGS,       NULL },
  { RCS_ACTION_GET_ALLOWED_TRANSFORMS,RCS_ACTION_ARG_INSTANCE_ID ACTION_GET_ARGS(RCS_VAR_ALLOWED_TRANSFORM_SETTINGS),       NULL },
  { RCS_ACTION_GET_TRANSFORMS,RCS_ACTION_GET_TRANSFORMS_ARGS,       NULL },
  { RCS_ACTION_SET_TRANSFORMS,RCS_ACTION_SET_TRANSFORMS_ARGS,       NULL },
  { RCS_ACTION_GET_ALLOWED_DEFAULT_TRANSFORMS,RCS_ACTION_GET_ALLOWED_DEFAULT_TRANSFORMS_ARGS,       NULL },
  { RCS_ACTION_GET_DEFAULT_TRANSFORMS,RCS_ACTION_GET_DEFAULT_TRANSFORMS_ARGS,       NULL },
  { RCS_ACTION_SET_DEFAULT_TRANSFORMS,RCS_ACTION_SET_DEFAULT_TRANSFORMS_ARGS,       NULL },
  { RCS_ACTION_GET_ALL_AVAILABLE_TRANSFORMS,RCS_ACTION_GET_ALL_AVAILABLE_TRANSFORMS_ARGS,       NULL },
  { NULL, NULL,                                  NULL }
};

upnp_service_statevar_t rcs_service_variables[] = {
  { RCS_VAR_LAST_CHANGE, E_STRING, 0, NULL},
  { RCS_VAR_PRESET_NAME_LIST, E_STRING, 0, NULL},
  { RCS_VAR_BRIGHTNESS, E_UI2, 0, NULL},
  { RCS_VAR_CONTRAST, E_UI2, 0, NULL},
  { RCS_VAR_SHARPNESS, E_UI2, 0, NULL},
  { RCS_VAR_RED_VIDEO_GAIN, E_UI2, 0, NULL},
  { RCS_VAR_GREEN_VIDEO_GAIN, E_UI2, 0, NULL},
  { RCS_VAR_BLUE_VIDEO_GAIN, E_UI2, 0, NULL},
  { RCS_VAR_RED_VIDEO_BLACK_LEVEL, E_UI2, 0, NULL},
  { RCS_VAR_GREEN_VIDEO_BLACK_LEVEL, E_UI2, 0, NULL},
  { RCS_VAR_BLUE_VIDEO_BLACK_LEVEL, E_UI2, 0, NULL},
  { RCS_VAR_COLOR_TEMPERATURE, E_UI2, 0, NULL},
  { RCS_VAR_HORIZONTAL_KEYSTONE, E_I2, 0, NULL},
  { RCS_VAR_VERTICAL_KEYSTONE, E_I2, 0, NULL},
  { RCS_VAR_MUTE, E_BOOLEAN, 0, NULL},
  { RCS_VAR_VOLUME, E_UI2, 0, NULL},
  { RCS_VAR_VOLUME_DB, E_I2, 0, NULL},
  { RCS_VAR_LOUDNESS, E_BOOLEAN, 0, NULL},
  { RCS_VAR_ALLOWED_TRANSFORM_SETTINGS, E_STRING, 0, NULL},
  { RCS_VAR_TRANSFORM_SETTINGS, E_STRING, 0, NULL},
  { RCS_VAR_ALLOWED_DEFAULT_TRANSFORM_SETTINGS, E_STRING, 0, NULL},
  { RCS_VAR_DEFAULT_TRANSFORM_SETTINGS, E_STRING, 0, NULL},
  { RCS_VAR_A_ARG_TYPE_CHANNEL, E_STRING, 0, NULL},
  { RCS_VAR_A_ARG_TYPE_INSTANCE_ID, E_UI4, 0, NULL},
  { RCS_VAR_A_ARG_TYPE_PRESET_NAME, E_STRING, 0, NULL},
  { RCS_VAR_A_ARG_TYPE_DEVICE_UDN, E_STRING, 0, NULL},
  { RCS_VAR_A_ARG_TYPE_SERVICE_TYPE, E_STRING, 0, NULL},
  { RCS_VAR_A_ARG_TYPE_SERVICE_ID, E_STRING, 0, NULL},
  { RCS_VAR_A_ARG_TYPE_STATE_VARIABLE_VALUE_PAIRS, E_STRING, 0, NULL},
  { RCS_VAR_A_ARG_TYPE_STATE_VARIABLE_LIST, E_STRING, 0, NULL},
  { NULL, 0, 0, NULL}, 
};

static char *
rcs_get_description (dlna_t *dlna)
{
  return dlna_service_get_description (dlna, rcs_service_actions, rcs_service_variables);
}

dlna_service_t *
rcs_service_new (dlna_t *dlna dlna_unused)
{
  dlna_service_t *service = NULL;
  service = calloc (1, sizeof (dlna_service_t));
  
  service->id           = RCS_SERVICE_ID;
  service->type         = RCS_SERVICE_TYPE;
  service->scpd_url     = RCS_URL;
  service->control_url  = RCS_CONTROL_URL;
  service->event_url    = RCS_EVENT_URL;
  service->actions      = rcs_service_actions;
  service->statevar     = rcs_service_variables;
  service->get_description     = rcs_get_description;
  service->init         = NULL;
  service->last_change  = 1;

  return service;
};
