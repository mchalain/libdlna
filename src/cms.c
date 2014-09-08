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

#include <stdio.h>
#include <stdlib.h>

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
  char **mimes;
  buffer_t *source;
  
  if (!dlna || !ev)
    return 0;

  source = buffer_new ();
  mimes = dlna->cms.sourcemimes;

  while (*mimes)
  {
    /* we do only support HTTP right now */
    /* format for protocol info is:
     *  <protocol>:<network>:<contentFormat>:<additionalInfo>
     */
    buffer_appendf (source, "http-get:*:%s:*", *mimes++);
    if (*mimes)
      buffer_append (source, ",");
  }

  upnp_add_response (ev, CMS_ARG_SOURCE, source->buf);
  buffer_free (source);

  source = buffer_new ();
  mimes = dlna->cms.sinkmimes;

  while (*mimes)
  {
    /* we do only support HTTP right now */
    /* format for protocol info is:
     *  <protocol>:<network>:<contentFormat>:<additionalInfo>
     */
    buffer_appendf (source, "http-get:*:%s:*", *mimes++);
    if (*mimes)
      buffer_append (source, ",");
  }

  upnp_add_response (ev, CMS_ARG_SINK, source->buf);
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
  char **mimes;
  
  if (!dlna || !ev)
    return 0;

  upnp_add_response (ev, CMS_ARG_CONNECTION_ID,
                     CMS_DEFAULT_CON_ID);
  upnp_add_response (ev, CMS_ARG_RCS_ID, CMS_UNKNOW_ID);
  upnp_add_response (ev, CMS_ARG_TRANSPORT_ID, CMS_UNKNOW_ID);

  mimes = dlna->cms.sourcemimes;

  while (*mimes)
  {
    char protocol[512];

    memset (protocol, '\0', sizeof (protocol));
    snprintf (protocol, sizeof (protocol), "http-get:*:%s:*", *mimes++);
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

upnp_service_statevar_t cms_service_variables[] = {
  { "SourceProtocolInfo", E_STRING, 1, NULL},
  { "SinkProtocolInfo", E_STRING, 1, NULL},
  { "CurrentConnectionIDs", E_STRING, 1, NULL},
  { "FeatureList", E_STRING, 0, NULL},
  { "A_ARG_TYPE_ConnectionStatus", E_STRING, 0, NULL},
  { "A_ARG_TYPE_ConnectionManager", E_STRING, 0, NULL},
  { "A_ARG_TYPE_Direction", E_STRING, 0, NULL},
  { "A_ARG_TYPE_ProtocolInfo", E_STRING, 0, NULL},
  { "A_ARG_TYPE_ConnectionID", E_I4, 0, NULL},
  { "A_ARG_TYPE_AVTransportID", E_I4, 0, NULL},
  { "A_ARG_TYPE_RcsID", E_I4, 0, NULL},
  { NULL, 0, 0, NULL},
};

static char *
cms_get_description (dlna_t *dlna)
{
  return dlna_service_get_description (dlna, cms_service_actions, cms_service_variables);
}

dlna_service_t *
cms_service_new (dlna_t *dlna dlna_unused)
{
  dlna_service_t *service = NULL;
  service = calloc (1, sizeof (dlna_service_t));
  
  service->id           = CMS_SERVICE_ID;
  service->type         = CMS_SERVICE_TYPE;
  service->scpd_url     = CMS_URL;
  service->control_url  = CMS_CONTROL_URL;
  service->event_url    = CMS_EVENT_URL;
  service->actions      = cms_service_actions;
  service->statevar     = cms_service_variables;
  service->get_description     = cms_get_description;
  service->init         = NULL;
  service->last_change  = 1;

  return service;
};
