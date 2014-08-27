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

#include <stdlib.h>

#include "upnp_internals.h"

#define MSR_DESCRIPTION \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">" \
"<specVersion>" \
"  <major>1</major>" \
"  <minor>0</minor>" \
"</specVersion>" \
"<actionList>" \
"  <action>" \
"    <name>IsAuthorized</name>" \
"    <argumentList>" \
"      <argument>" \
"        <name>DeviceID</name>" \
"        <direction>in</direction>" \
"        <relatedStateVariable>A_ARG_TYPE_DeviceID</relatedStateVariable>" \
"      </argument>" \
"      <argument>" \
"        <name>Result</name>" \
"        <direction>out</direction>" \
"        <relatedStateVariable>A_ARG_TYPE_Result</relatedStateVariable>" \
"      </argument>" \
"    </argumentList>" \
"  </action>" \
"  <action>" \
"    <name>RegisterDevice</name>" \
"    <argumentList>" \
"      <argument>" \
"        <name>RegistrationReqMsg</name>" \
"        <direction>in</direction>" \
"        <relatedStateVariable>A_ARG_TYPE_RegistrationReqMsg</relatedStateVariable>" \
"      </argument>" \
"      <argument>" \
"        <name>RegistrationRespMsg</name>" \
"        <direction>out</direction>" \
"        <relatedStateVariable>A_ARG_TYPE_RegistrationRespMsg</relatedStateVariable>" \
"      </argument>" \
"    </argumentList>" \
"  </action>" \
"  <action>" \
"    <name>IsValidated</name>" \
"    <argumentList>" \
"      <argument>" \
"        <name>DeviceID</name>" \
"        <direction>in</direction>" \
"        <relatedStateVariable>A_ARG_TYPE_DeviceID</relatedStateVariable>" \
"      </argument>" \
"      <argument>" \
"        <name>Result</name>" \
"        <direction>out</direction>" \
"        <relatedStateVariable>A_ARG_TYPE_Result</relatedStateVariable>" \
"      </argument>" \
"    </argumentList>" \
"  </action>" \
"</actionList>" \
"<serviceStateTable>" \
"  <stateVariable sendEvents=\"no\">" \
"    <name>A_ARG_TYPE_DeviceID</name>" \
"    <dataType>string</dataType>" \
"  </stateVariable>" \
"  <stateVariable sendEvents=\"no\">" \
"    <name>A_ARG_TYPE_Result</name>" \
"    <dataType>int</dataType>" \
"  </stateVariable>" \
"  <stateVariable sendEvents=\"no\">" \
"    <name>A_ARG_TYPE_RegistrationReqMsg</name>" \
"    <dataType>bin.base64</dataType>" \
"  </stateVariable>" \
"  <stateVariable sendEvents=\"no\">" \
"    <name>A_ARG_TYPE_RegistrationRespMsg</name>" \
"    <dataType>bin.base64</dataType>" \
"  </stateVariable>" \
"  <stateVariable sendEvents=\"no\">" \
"    <name>AuthorizationGrantedUpdateID</name>" \
"    <dataType>ui4</dataType>" \
"  </stateVariable>" \
"  <stateVariable sendEvents=\"no\">" \
"    <name>AuthorizationDeniedUpdateID</name>" \
"    <dataType>ui4</dataType>" \
"  </stateVariable>" \
"  <stateVariable sendEvents=\"no\">" \
"    <name>ValidationSucceededUpdateID</name>" \
"    <dataType>ui4</dataType>" \
"  </stateVariable>" \
"  <stateVariable sendEvents=\"no\">" \
"    <name>ValidationRevokedUpdateID</name>" \
"    <dataType>ui4</dataType>" \
"  </stateVariable>" \
"</serviceStateTable>" \
"</scpd>"

/* MSR Action Names */
#define SERVICE_MSR_ACTION_IS_AUTHORIZED            "IsAuthorized"
#define SERVICE_MSR_ACTION_REGISTER_DEVICE          "RegisterDevice"
#define SERVICE_MSR_ACTION_IS_VALIDATED             "IsValidated"

/* MSR Arguments */
#define SERVICE_MSR_ARG_DEVICE_ID                   "DeviceID"
#define SERVICE_MSR_ARG_RESULT                      "Result"
#define SERVICE_MSR_ARG_REGISTRATION_REQUEST_MSG    "RegistrationReqMsg"
#define SERVICE_MSR_ARG_REGISTRATION_RESPONSE_MSG   "RegistrationRespMsg"

/* MSR Argument Values */
#define SERVICE_MSR_STATUS_OK                       "1"

char *
msr_get_description (dlna_t *dlna)
{
  return strdup (MSR_DESCRIPTION);
}

static int
msr_is_authorized (dlna_t *dlna, upnp_action_event_t *ev)
{
  if (!dlna || !ev)
    return 0;

  /* send a fake authorization to these stupid MS players ;-) */
  upnp_add_response (ev, SERVICE_MSR_ARG_RESULT, SERVICE_MSR_STATUS_OK);

  return ev->status;
}

static int
msr_register_device (dlna_t *dlna, upnp_action_event_t *ev)
{
  if (!dlna || !ev)
    return 0;

  /* no action is needed */

  return ev->status;
}

static int
msr_is_validated (dlna_t *dlna, upnp_action_event_t *ev)
{
  if (!dlna || !ev)
    return 0;

  /* send a fake validation to these stupid MS players ;-) */
  upnp_add_response (ev, SERVICE_MSR_ARG_RESULT, SERVICE_MSR_STATUS_OK);

  return ev->status;
}

/* List of UPnP Microsoft Registrar Service actions */
static upnp_service_action_t msr_service_actions[] = {
  { SERVICE_MSR_ACTION_IS_AUTHORIZED,   msr_is_authorized },
  { SERVICE_MSR_ACTION_REGISTER_DEVICE, msr_register_device },
  { SERVICE_MSR_ACTION_IS_VALIDATED,    msr_is_validated },
  { NULL,                               NULL }
};

upnp_service_t msr_service = {
  .id           = MSR_SERVICE_ID,
  .type         = MSR_SERVICE_TYPE,
  .scpd_url     = MSR_URL,
  .control_url  = MSR_CONTROL_URL,
  .event_url    = MSR_EVENT_URL,
  .actions      = msr_service_actions,
  .get_description     = msr_get_description,
};
