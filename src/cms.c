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
 * ConnectionManager service specifications can be found at:
 * http://upnp.org/standardizeddcps/documents/ConnectionManager1.0.pdf
 * http://upnp.org/specs/av/UPnP-av-ConnectionManager-v2-Service-20060531.pdf
 */

#include "upnp_internals.h"
#include "services.h"
#include "cms.h"

#define CMS_ARG_CONNECTION_STATUS_ALLOWED \
"      <allowedValueList>" \
"        <allowedValue>OK</allowedValue>" \
"        <allowedValue>ContentFormatMismatch</allowedValue>" \
"        <allowedValue>InsufficientBandwidth</allowedValue>" \
"        <allowedValue>UnreliableChannel</allowedValue>" \
"        <allowedValue>Unknown</allowedValue>" \
"      </allowedValueList>"
#define CMS_ARG_DIRECTION_ALLOWED \
"      <allowedValueList>" \
"        <allowedValue>Input</allowedValue>" \
"        <allowedValue>Output</allowedValue>" \
"      </allowedValueList>"

/* CMS Action Names */
#define CMS_ACTION_PROT_INFO          "GetProtocolInfo"
#define CMS_ACTION_PROT_INFO_ARGS \
ACTION_ARG_OUT(CMS_ARG_SOURCE,"SourceProtocolInfo") \
ACTION_ARG_OUT(CMS_ARG_SINK,"SinkProtocolInfo")

#define CMS_ACTION_PREPARE            "PrepareForConnection"
#define CMS_ACTION_CON_COMPLETE       "ConnectionComplete"
#define CMS_ACTION_CON_ID             "GetCurrentConnectionIDs"
#define CMS_ACTION_CON_ID_ARGS \
ACTION_ARG_OUT(CMS_ARG_CONNECTION_IDS,"CurrentConnectionIDs")

#define CMS_ACTION_CON_INFO           "GetCurrentConnectionInfo"
#define CMS_ACTION_CON_INFO_ARGS \
ACTION_ARG_OUT(CMS_ARG_CONNECTION_ID,"A_ARG_TYPE_ConnectionID") \
ACTION_ARG_OUT(CMS_ARG_RCS_ID,"A_ARG_TYPE_RcsID") \
ACTION_ARG_OUT(CMS_ARG_TRANSPORT_ID,"A_ARG_TYPE_AVTransportID") \
ACTION_ARG_OUT(CMS_ARG_PROT_INFO,"A_ARG_TYPE_ProtocolInfo") \
ACTION_ARG_OUT(CMS_ARG_PEER_CON_MANAGER,"A_ARG_TYPE_ConnectionManager") \
ACTION_ARG_OUT(CMS_ARG_PEER_CON_ID,"A_ARG_TYPE_ConnectionID") \
ACTION_ARG_OUT(CMS_ARG_DIRECTION,"A_ARG_TYPE_Direction") \
ACTION_ARG_OUT(CMS_ARG_STATUS,"A_ARG_TYPE_ConnectionStatus") \


/* CMS Arguments */
#define CMS_ARG_SOURCE                "Source"
#define CMS_ARG_SINK                  "Sink"
#define CMS_ARG_CONNECTION_IDS        "ConnectionIDs"
#define CMS_ARG_CONNECTION_ID         "ConnectionID"
#define CMS_ARG_RCS_ID                "RcsID"
#define CMS_ARG_TRANSPORT_ID          "AVTransportID"
#define CMS_ARG_PROT_INFO             "ProtocolInfo"
#define CMS_ARG_PEER_CON_MANAGER      "PeerConnectionManager"
#define CMS_ARG_PEER_CON_ID           "PeerConnectionID"
#define CMS_ARG_DIRECTION             "Direction"
#define CMS_ARG_STATUS                "Status"

/* CMS Argument Values */
#define CMS_DEFAULT_CON_ID            "0"
#define CMS_UNKNOW_ID                 "-1"
#define CMS_OUTPUT                    "Output"
#define CMS_STATUS_OK                 "OK"

/* CMS Error Codes */
#define CMS_ERR_INVALID_ARGS          402
#define CMS_ERR_PARAMETER_MISMATCH    706

/*
 * GetProtocolInfo:
 *   Returns the protocol-related info that this ConnectionManager supports in
 *   its current state, as a comma-separate list of strings.
 */
static int
cms_get_protocol_info (dlna_t *dlna, upnp_action_event_t *ev)
{
  char **mimes, **tmp;
  buffer_t *source;
  
  if (!dlna || !ev)
    return 0;

  source = buffer_new ();
  mimes = dlna_get_supported_mime_types (dlna);
  tmp = mimes;

  while (*tmp)
  {
    /* we do only support HTTP right now */
    /* format for protocol info is:
     *  <protocol>:<network>:<contentFormat>:<additionalInfo>
     */
    buffer_appendf (source, "http-get:*:%s:*", *tmp++);
    if (*tmp)
      buffer_append (source, ",");
  }

  upnp_add_response (ev, CMS_ARG_SOURCE, source->buf);
  upnp_add_response (ev, CMS_ARG_SINK, "");
  
  buffer_free (source);
  
  return ev->status;
}

static int
cms_get_current_connection_ids (dlna_t *dlna, upnp_action_event_t *ev)
{
  if (!dlna || !ev)
    return 0;

  upnp_add_response (ev, CMS_ARG_CONNECTION_IDS, "");
  
  return ev->status;
}

static int
cms_get_current_connection_info (dlna_t *dlna, upnp_action_event_t *ev)
{
  char **mimes, **tmp;
  
  if (!dlna || !ev)
    return 0;

  upnp_add_response (ev, CMS_ARG_CONNECTION_ID,
                     CMS_DEFAULT_CON_ID);
  upnp_add_response (ev, CMS_ARG_RCS_ID, CMS_UNKNOW_ID);
  upnp_add_response (ev, CMS_ARG_TRANSPORT_ID, CMS_UNKNOW_ID);

  mimes = dlna_get_supported_mime_types (dlna);
  tmp = mimes;

  while (*tmp)
  {
    char protocol[512];

    memset (protocol, '\0', sizeof (protocol));
    snprintf (protocol, sizeof (protocol), "http-get:*:%s:*", *tmp++);
    upnp_add_response (ev, CMS_ARG_PROT_INFO, protocol);
  }
  
  upnp_add_response (ev, CMS_ARG_PEER_CON_MANAGER, "");
  upnp_add_response (ev, CMS_ARG_PEER_CON_ID, CMS_UNKNOW_ID);
  upnp_add_response (ev, CMS_ARG_DIRECTION, CMS_OUTPUT);
  upnp_add_response (ev, CMS_ARG_STATUS, CMS_STATUS_OK);
  
  return ev->status;
}

/* List of UPnP ConnectionManager Service actions */
static upnp_service_action_t cms_service_actions[] = {
  { CMS_ACTION_PROT_INFO, CMS_ACTION_PROT_INFO_ARGS,    cms_get_protocol_info },
  { CMS_ACTION_PREPARE, NULL,      NULL },
  { CMS_ACTION_CON_COMPLETE, NULL,  NULL },
  { CMS_ACTION_CON_ID, CMS_ACTION_CON_ID_ARGS,        cms_get_current_connection_ids },
  { CMS_ACTION_CON_INFO, CMS_ACTION_CON_INFO_ARGS,      cms_get_current_connection_info },
  { NULL, NULL,                            NULL }
};

upnp_service_variable_t cms_service_variables[] = {
  { "SourceProtocolInfo", E_STRING, 1},
  { "SinkProtocolInfo", E_STRING, 1},
  { "CurrentConnectionIDs", E_STRING, 1},
  { "FeatureList", E_STRING, 0},
  { "A_ARG_TYPE_ConnectionStatus", E_STRING, 0},
  { "A_ARG_TYPE_ConnectionManager", E_STRING, 0},
  { "A_ARG_TYPE_Direction", E_STRING, 0},
  { "A_ARG_TYPE_ProtocolInfo", E_STRING, 0},
  { "A_ARG_TYPE_ConnectionID", E_I4, 0},
  { "A_ARG_TYPE_AVTransportID", E_I4, 0},
  { "A_ARG_TYPE_RcsID", E_I4, 0},
  { NULL, 0, 0},
};

static char *
cms_get_description (dlna_t *dlna)
{
  return dlna_service_get_description (dlna, cms_service_actions, cms_service_variables);
}

upnp_service_t cms_service = {
  .id           = CMS_SERVICE_ID,
  .type         = CMS_SERVICE_TYPE,
  .scpd_url     = CMS_URL,
  .control_url  = CMS_CONTROL_URL,
  .event_url    = CMS_EVENT_URL,
  .actions      = cms_service_actions,
  .get_description     = cms_get_description,
};

