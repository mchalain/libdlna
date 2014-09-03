/*
 * libdlna: reference DLNA standards implementation.
 * Copyright (C) 2008 Benjamin Zores <ben@geexbox.org>
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dlna_internals.h"
#include "upnp_internals.h"
#include "services.h"
#include "devices.h"
#include "cms.h"
#include "cds.h"
#include "avts.h"
#include "msr.h"
#include "rcs.h"

#define SERVICE_HEADER \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">" \
"  <specVersion>" \
"     <major>1</major>" \
"     <minor>0</minor>" \
"  </specVersion>"
#define SERVICE_FOOTER \
"</scpd>"

#define SERVICE_ACTIONLIST_HEADER \
"  <actionList>"
#define SERVICE_ACTIONLIST_FOOTER \
"  </actionList>"

#define SERVICE_ACTION_DEF \
"    <action>" \
"      <name>%s</name>" \
"      <argumentList>" \
"%s" \
"      </argumentList>" \
"    </action>"

#define SERVICE_STATETABLE_HEADER \
"  <serviceStateTable>"
#define SERVICE_STATETABLE_FOOTER \
"  </serviceStateTable>"

#define SERVICE_STATEVARIABLE_DEF \
"    <stateVariable sendEvents=\"%s\">" \
"      <name>%s</name>" \
"      <dataType>%s</dataType>" \
"    </stateVariable>"

char *SERVICE_STATE_EVENTING[] = {
  "no",
  "yes"
};
char *SERVICE_STATE_TYPES[] = {
  STRING,
  BOOLEAN,
  I2,
  UI2,
  I4,
  UI4,
  URI,
};

char *
dlna_service_get_description (dlna_t *dlna dlna_unused, upnp_service_action_t *actions, upnp_service_statevar_t *variables)
{
  buffer_t *b = NULL;
  char *desc = NULL;

  b = buffer_new ();
  
  buffer_appendf (b, SERVICE_HEADER);
  if (actions)
  {
    buffer_appendf (b, SERVICE_ACTIONLIST_HEADER);
    while (actions->name)
    {
      if (actions->cb)
        buffer_appendf (b, SERVICE_ACTION_DEF, actions->name, actions->args?actions->args:"");
      actions++;
    }
    buffer_appendf (b, SERVICE_ACTIONLIST_FOOTER);
  }
  if (variables)
  {
    buffer_appendf (b, SERVICE_STATETABLE_HEADER);
    while (variables->name)
    {
      char *eventing = SERVICE_STATE_EVENTING[variables->eventing];
      char *type = SERVICE_STATE_TYPES[variables->type];
      buffer_appendf (b, SERVICE_STATEVARIABLE_DEF, eventing, variables->name, type);
      variables++;
    }
    buffer_appendf (b, SERVICE_STATETABLE_FOOTER);
  }
  buffer_appendf (b, SERVICE_FOOTER);

  desc = strdup (b->buf);
  buffer_free (b);
  
  return desc;
}

void
dlna_service_register (dlna_device_t *device, dlna_service_t *service)
{
  dlna_service_list_t *item = NULL;
  if (!device || !service)
    return;

  item = calloc (1, sizeof(dlna_service_list_t));

  item->id = service->typeid;
  item->service = calloc (1, sizeof (dlna_service_t));
  memcpy (item->service, service, sizeof (dlna_service_t));
  HASH_ADD_INT (device->services, id, item);
}

static void
dlna_service_free (dlna_device_t *device, dlna_service_list_t *item)
{
  if (!device || !device->services || !item)
    return;

  HASH_DEL (device->services, item);
  free (item->service);
  free (item);
}

dlna_service_t *
dlna_service_find (dlna_device_t *device, char *id)
{
  dlna_service_list_t *item;

  if (!device || !device->services || !id)
    return NULL;

  for (item = device->services; item; item = item->hh.next)
    if (item->service->id && id && !strcmp (item->service->id, id))
      return item->service;

  return NULL;
}

dlna_service_t *
dlna_service_find_url (dlna_device_t *device, char *url)
{
  dlna_service_list_t *item;

  if (!device || !device->services || !url)
    return NULL;

  for (item = device->services; item; item = item->hh.next)
    if (item->service->scpd_url && url && !strcmp (item->service->scpd_url, url))
      return item->service;

  return NULL;
}

int
dlna_service_foreach (dlna_device_t *device, int (*cb)(void *cookie, dlna_service_t *service), void *cookie)
{
  int ret = 0;
  dlna_service_list_t *item;

  if (!device || !device->services || !cb)
    return -1;

  for (item = device->services; item; item = item->hh.next)
  {
    ret = cb (cookie, item->service);
    if (ret)
      break;
  }
  return ret;
}

void
dlna_service_unregister (dlna_device_t *device, dlna_service_t *service)
{
  int id;
  dlna_service_list_t *item = NULL;

  if (!device || !device->services || !service)
    return;

  id = service->typeid;
  HASH_FIND_INT (device->services, &id, item);

  dlna_service_free (device, item);
}

void
dlna_service_unregister_all (dlna_device_t *device)
{
  dlna_service_list_t *item;

  if (!device || !device->services)
    return;

  for (item = device->services; item; item = item->hh.next)
    dlna_service_free (device, item);
  device->services = NULL;
}
