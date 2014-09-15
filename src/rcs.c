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

#define RCS_ERR_ACTION_FAILED                 501
#define RCS_ERR_INVALID_INSTANCE              718 

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

#define RCS_ARG_INSTANCEID            "InstanceID"
#define RCS_ARG_CUR_PRESET_NAME_LIST  "Current"RCS_VAR_PRESET_NAME_LIST

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

typedef struct rcs_instance_s rcs_instance_t;
struct rcs_instance_s
{
  uint32_t id;
  dlna_service_t *service;
  UT_hash_handle hh;
};

static rcs_instance_t *
rcs_create_instance (dlna_service_t *service, uint32_t id)
{
  rcs_instance_t *instance = NULL;
  rcs_instance_t *instances = (rcs_instance_t *)service->cookie;

  instance = calloc (1, sizeof(rcs_instance_t));

  instance->id = id;
  instance->service = service;
  HASH_ADD_INT (instances, id, instance);
  service->cookie = instances;
  return instance;
}

static void
rcs_kill_instance (dlna_service_t *service, uint32_t instanceID)
{
  rcs_instance_t *instance = NULL;
  rcs_instance_t *instances = (rcs_instance_t *)service->cookie;

  HASH_FIND_INT (instances, &instanceID, instance);

  if (instance)
  {
    HASH_DEL (instances, instance);
    free (instance);
  }
  return;
}

static int
rcs_list_presets (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t instanceID;
  rcs_instance_t *instance = NULL;
  rcs_instance_t *instances = (rcs_instance_t *)ev->service->cookie;

  if (!dlna || !ev)
  {
    ev->ar->ErrCode = RCS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Check for status */
  if (!ev->status)
  {
    ev->ar->ErrCode = RCS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Retrieve input arguments */
  instanceID   = upnp_get_ui4 (ev->ar, RCS_ARG_INSTANCEID);
  HASH_FIND_INT (instances, &instanceID, instance);
  if (!instance)
  {
    ev->ar->ErrCode = RCS_ERR_INVALID_INSTANCE;
    return 0;
  }
  upnp_add_response (ev, RCS_ARG_CUR_PRESET_NAME_LIST, "default");

  return ev->status;
}

static int
rcs_select_preset (dlna_t *dlna, upnp_action_event_t *ev)
{
  uint32_t instanceID;
  rcs_instance_t *instance = NULL;
  rcs_instance_t *instances = (rcs_instance_t *)ev->service->cookie;

  if (!dlna || !ev)
  {
    ev->ar->ErrCode = RCS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Check for status */
  if (!ev->status)
  {
    ev->ar->ErrCode = RCS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Retrieve input arguments */
  instanceID   = upnp_get_ui4 (ev->ar, RCS_ARG_INSTANCEID);
  HASH_FIND_INT (instances, &instanceID, instance);
  if (!instance)
  {
    ev->ar->ErrCode = RCS_ERR_INVALID_INSTANCE;
    return 0;
  }

  return ev->status;
}

/* List of UPnP Rendering Control Service actions */
upnp_service_action_t rcs_service_actions[] = {
  { RCS_ACTION_LIST_PRESETS,
    .args = RCS_ACTION_LIST_PRESETS_ARGS,
    .cb = rcs_list_presets },
  { RCS_ACTION_SELECT_PRESET,
    .args = RCS_ACTION_SELECT_PRESET_ARGS,
    .cb = rcs_select_preset },
  { RCS_ACTION_GET_BRIGHTNESS,
    .args = ACTION_GET_ARGS(RCS_VAR_BRIGHTNESS),
    .cb = NULL },
  { RCS_ACTION_SET_BRIGHTNESS,
    .args = ACTION_SET_ARGS(RCS_VAR_BRIGHTNESS),
    .cb = NULL },
  { RCS_ACTION_GET_CONTRAST,
    .args = ACTION_GET_ARGS(RCS_VAR_CONTRAST),
    .cb = NULL },
  { RCS_ACTION_SET_CONTRAST,
    .args = ACTION_SET_ARGS(RCS_VAR_CONTRAST),
    .cb = NULL },
  { RCS_ACTION_GET_SHARPNESS,
    .args = ACTION_GET_ARGS(RCS_VAR_SHARPNESS),
    .cb = NULL },
  { RCS_ACTION_SET_SHARPNESS,
    .args = ACTION_SET_ARGS(RCS_VAR_SHARPNESS),
    .cb = NULL },
  { RCS_ACTION_GET_R_V_GAIN,
    .args = ACTION_GET_ARGS(RCS_VAR_RED_VIDEO_GAIN),
    .cb = NULL },
  { RCS_ACTION_SET_R_V_GAIN,
    .args = ACTION_SET_ARGS(RCS_VAR_RED_VIDEO_GAIN),
    .cb = NULL },
  { RCS_ACTION_GET_G_V_GAIN,
    .args = ACTION_GET_ARGS(RCS_VAR_GREEN_VIDEO_GAIN),
    .cb = NULL },
  { RCS_ACTION_SET_G_V_GAIN,
    .args = ACTION_SET_ARGS(RCS_VAR_GREEN_VIDEO_GAIN),
    .cb = NULL },
  { RCS_ACTION_GET_B_V_GAIN,
    .args = ACTION_GET_ARGS(RCS_VAR_BLUE_VIDEO_GAIN),
    .cb = NULL },
  { RCS_ACTION_SET_B_V_GAIN,
    .args = ACTION_SET_ARGS(RCS_VAR_BLUE_VIDEO_GAIN),
    .cb = NULL },
  { RCS_ACTION_GET_R_V_BLEVEL,
    .args = ACTION_GET_ARGS(RCS_VAR_RED_VIDEO_BLACK_LEVEL),
    .cb = NULL },
  { RCS_ACTION_SET_R_V_BLEVEL,
    .args = ACTION_SET_ARGS(RCS_VAR_RED_VIDEO_BLACK_LEVEL),
    .cb = NULL },
  { RCS_ACTION_GET_G_V_BLEVEL,
    .args = ACTION_GET_ARGS(RCS_VAR_GREEN_VIDEO_BLACK_LEVEL),
    .cb = NULL },
  { RCS_ACTION_SET_G_V_BLEVEL,
    .args = ACTION_SET_ARGS(RCS_VAR_GREEN_VIDEO_BLACK_LEVEL),
    .cb = NULL },
  { RCS_ACTION_GET_B_V_BLEVEL,
    .args = ACTION_GET_ARGS(RCS_VAR_BLUE_VIDEO_BLACK_LEVEL),
    .cb = NULL },
  { RCS_ACTION_SET_B_V_BLEVEL,
    .args = ACTION_SET_ARGS(RCS_VAR_BLUE_VIDEO_BLACK_LEVEL),
    .cb = NULL },
  { RCS_ACTION_GET_COLOR_TEMPERATURE,
    .args = ACTION_GET_ARGS(RCS_VAR_COLOR_TEMPERATURE),
    .cb = NULL },
  { RCS_ACTION_SET_COLOR_TEMPERATURE, 
    .args = ACTION_SET_ARGS(RCS_VAR_COLOR_TEMPERATURE),
    .cb = NULL },
  { RCS_ACTION_GET_HORIZONTAL_KEYSTONE,
    .args = ACTION_GET_ARGS(RCS_VAR_HORIZONTAL_KEYSTONE),
    .cb = NULL },
  { RCS_ACTION_SET_HORIZONTAL_KEYSTONE,
    .args = ACTION_SET_ARGS(RCS_VAR_HORIZONTAL_KEYSTONE),
    .cb = NULL },
  { RCS_ACTION_GET_VERTICAL_KEYSTONE,
    .args = ACTION_GET_ARGS(RCS_VAR_VERTICAL_KEYSTONE),
    .cb = NULL },
  { RCS_ACTION_SET_VERTICAL_KEYSTONE,
    .args = ACTION_SET_ARGS(RCS_VAR_VERTICAL_KEYSTONE),
    .cb = NULL },
  { RCS_ACTION_GET_MUTE,
    .args = ACTION_GET_ARGS(RCS_VAR_MUTE),
    .cb = NULL },
  { RCS_ACTION_SET_MUTE,
    .args = ACTION_SET_ARGS(RCS_VAR_MUTE),
    .cb = NULL },
  { RCS_ACTION_GET_VOLUME,
    .args = ACTION_GET_ARGS(RCS_VAR_VOLUME),
    .cb = NULL },
  { RCS_ACTION_SET_VOLUME,
    .args = ACTION_SET_ARGS(RCS_VAR_VOLUME),
    .cb = NULL },
  { RCS_ACTION_GET_VOLUME_DB,
    .args = ACTION_GET_ARGS(RCS_VAR_VOLUME_DB),
    .cb = NULL },
  { RCS_ACTION_SET_VOLUME_DB,
    .args = ACTION_SET_ARGS(RCS_VAR_VOLUME_DB),
    .cb = NULL },
  { RCS_ACTION_GET_VOLUME_DB_RANGE,
    .args =RCS_ACTION_GET_VOLUME_DB_RANGE_ARGS,
    .cb = NULL },
  { RCS_ACTION_GET_LOUDNESS,
    .args = ACTION_GET_ARGS(RCS_VAR_LOUDNESS),
    .cb = NULL },
  { RCS_ACTION_SET_LOUDNESS,
    .args = ACTION_SET_ARGS(RCS_VAR_LOUDNESS),
    .cb = NULL },
  { RCS_ACTION_GET_STATE_VARIABLES,
    .args = RCS_ACTION_GET_STATE_VARIABLES_ARGS,
    .cb = NULL },
  { RCS_ACTION_SET_STATE_VARIABLES,
    .args = RCS_ACTION_SET_STATE_VARIABLES_ARGS,
    .cb = NULL },
  { RCS_ACTION_GET_ALLOWED_TRANSFORMS,
    .args = RCS_ACTION_ARG_INSTANCE_ID ACTION_GET_ARGS(RCS_VAR_ALLOWED_TRANSFORM_SETTINGS),
    .cb = NULL },
  { RCS_ACTION_GET_TRANSFORMS,
    .args = RCS_ACTION_GET_TRANSFORMS_ARGS,
    .cb = NULL },
  { RCS_ACTION_SET_TRANSFORMS,
    .args =RCS_ACTION_SET_TRANSFORMS_ARGS,
    .cb = NULL },
  { RCS_ACTION_GET_ALLOWED_DEFAULT_TRANSFORMS,
    .args = RCS_ACTION_GET_ALLOWED_DEFAULT_TRANSFORMS_ARGS,
    .cb = NULL },
  { RCS_ACTION_GET_DEFAULT_TRANSFORMS,
    .args = RCS_ACTION_GET_DEFAULT_TRANSFORMS_ARGS,
    .cb = NULL },
  { RCS_ACTION_SET_DEFAULT_TRANSFORMS,
    .args = RCS_ACTION_SET_DEFAULT_TRANSFORMS_ARGS,
    .cb = NULL },
  { RCS_ACTION_GET_ALL_AVAILABLE_TRANSFORMS,
    .args = RCS_ACTION_GET_ALL_AVAILABLE_TRANSFORMS_ARGS,
    .cb = NULL },
  { NULL, NULL,
    .cb = NULL }
};

upnp_service_statevar_t rcs_service_variables[] = {
  { RCS_VAR_LAST_CHANGE, E_STRING, 0, NULL, NULL},
  { RCS_VAR_PRESET_NAME_LIST, E_STRING, 0, NULL, NULL},
  { RCS_VAR_BRIGHTNESS, E_UI2, 0, NULL, NULL},
  { RCS_VAR_CONTRAST, E_UI2, 0, NULL, NULL},
  { RCS_VAR_SHARPNESS, E_UI2, 0, NULL, NULL},
  { RCS_VAR_RED_VIDEO_GAIN, E_UI2, 0, NULL, NULL},
  { RCS_VAR_GREEN_VIDEO_GAIN, E_UI2, 0, NULL, NULL},
  { RCS_VAR_BLUE_VIDEO_GAIN, E_UI2, 0, NULL, NULL},
  { RCS_VAR_RED_VIDEO_BLACK_LEVEL, E_UI2, 0, NULL, NULL},
  { RCS_VAR_GREEN_VIDEO_BLACK_LEVEL, E_UI2, 0, NULL, NULL},
  { RCS_VAR_BLUE_VIDEO_BLACK_LEVEL, E_UI2, 0, NULL, NULL},
  { RCS_VAR_COLOR_TEMPERATURE, E_UI2, 0, NULL, NULL},
  { RCS_VAR_HORIZONTAL_KEYSTONE, E_I2, 0, NULL, NULL},
  { RCS_VAR_VERTICAL_KEYSTONE, E_I2, 0, NULL, NULL},
  { RCS_VAR_MUTE, E_BOOLEAN, 0, NULL, NULL},
  { RCS_VAR_VOLUME, E_UI2, 0, NULL, NULL},
  { RCS_VAR_VOLUME_DB, E_I2, 0, NULL, NULL},
  { RCS_VAR_LOUDNESS, E_BOOLEAN, 0, NULL, NULL},
  { RCS_VAR_ALLOWED_TRANSFORM_SETTINGS, E_STRING, 0, NULL, NULL},
  { RCS_VAR_TRANSFORM_SETTINGS, E_STRING, 0, NULL, NULL},
  { RCS_VAR_ALLOWED_DEFAULT_TRANSFORM_SETTINGS, E_STRING, 0, NULL, NULL},
  { RCS_VAR_DEFAULT_TRANSFORM_SETTINGS, E_STRING, 0, NULL, NULL},
  { RCS_VAR_A_ARG_TYPE_CHANNEL, E_STRING, 0, NULL, NULL},
  { RCS_VAR_A_ARG_TYPE_INSTANCE_ID, E_UI4, 0, NULL, NULL},
  { RCS_VAR_A_ARG_TYPE_PRESET_NAME, E_STRING, 0, NULL, NULL},
  { RCS_VAR_A_ARG_TYPE_DEVICE_UDN, E_STRING, 0, NULL, NULL},
  { RCS_VAR_A_ARG_TYPE_SERVICE_TYPE, E_STRING, 0, NULL, NULL},
  { RCS_VAR_A_ARG_TYPE_SERVICE_ID, E_STRING, 0, NULL, NULL},
  { RCS_VAR_A_ARG_TYPE_STATE_VARIABLE_VALUE_PAIRS, E_STRING, 0, NULL, NULL},
  { RCS_VAR_A_ARG_TYPE_STATE_VARIABLE_LIST, E_STRING, 0, NULL, NULL},
  { NULL, 0, 0, NULL, NULL}, 
};

static char *
rcs_get_description (dlna_service_t *service dlna_unused)
{
  return dlna_service_get_description (rcs_service_actions, rcs_service_variables);
}

static void
rcs_free (dlna_service_t *service)
{
  rcs_instance_t *instance;
  rcs_instance_t *instances = (rcs_instance_t *)service->cookie;

  for (instance = instances; instance; instance = instance->hh.next)
  {
    rcs_kill_instance (service, instance->id);
  }
}

dlna_service_t *
rcs_service_new (dlna_t *dlna dlna_unused)
{
  rcs_instance_t *instance;
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
  service->free         = rcs_free;
  service->last_change  = 1;

  instance = rcs_create_instance (service, 0);
  service->cookie = instance;

  return service;
};
