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

#ifndef SERVICES_H
#define SERVICES_H

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

typedef struct dlna_service_list_s dlna_service_list_t;

struct dlna_service_s {
  char *id;
  dlna_service_type_t typeid;
  char *type;
  char *scpd_url;
  char *control_url;
  char *event_url;
  struct upnp_service_action_s *actions;
  struct upnp_service_statevar_s *statevar;
  uint32_t last_change;
  void *cookie;
  char *(*get_description) (dlna_t *dlna);
  int (*init) (dlna_t *dlna);
};

struct dlna_service_list_s {
  dlna_service_type_t id;
  dlna_service_t *service;
  UT_hash_handle hh;
};

dlna_service_t *dlna_service_find (dlna_device_t *device, char *id);
dlna_service_t *dlna_service_find_id (dlna_device_t *device, uint32_t id);
dlna_service_t *dlna_service_find_url (dlna_device_t *device, char *url);
int dlna_service_foreach (dlna_device_t *device, int (*cb)(void *cookie, dlna_service_t *service), void *cookie);
void dlna_service_unregister_all (dlna_device_t *device);

char *
dlna_service_get_description (dlna_t *dlna, upnp_service_action_t *actions, upnp_service_statevar_t *variables);

#endif
