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
#include "cms.h"
#include "cds.h"
#include "avts.h"
#include "msr.h"

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
dlna_service_get_description (dlna_t *dlna, upnp_service_action_t *actions, upnp_service_variable_t *variables)
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
dlna_service_register (dlna_t *dlna, dlna_service_t *service)
{
  dlna_service_list_t *item = NULL;
  if (!dlna || !service)
    return;

  item = calloc (1, sizeof(dlna_service_list_t));

  item->id = service->typeid;
  item->service = calloc (1, sizeof (dlna_service_t));
  memcpy (item->service, service, sizeof (dlna_service_t));
  HASH_ADD_INT (dlna->services, id, item);
}

static void
dlna_service_free (dlna_t *dlna, dlna_service_list_t *item)
{
  if (!dlna || !item)
    return;

  HASH_DEL (dlna->services, item);
  free (item->service);
  free (item);
}

const dlna_service_t *
dlna_service_find (dlna_t *dlna, char *id)
{
  dlna_service_list_t *item;

  if (!dlna || !dlna->services || !id)
    return NULL;

  for (item = dlna->services; item; item = item->hh.next)
    if (item->service->id && id && !strcmp (item->service->id, id))
      return (const dlna_service_t *)item->service;

  return NULL;
}

const dlna_service_t *
dlna_service_find_url (dlna_t *dlna, char *url)
{
  dlna_service_list_t *item;

  if (!dlna || !dlna->services || !url)
    return NULL;

  for (item = dlna->services; item; item = item->hh.next)
    if (item->service->scpd_url && url && !strcmp (item->service->scpd_url, url))
      return (const dlna_service_t *)item->service;

  return NULL;
}

int
dlna_service_foreach (dlna_t *dlna, int (*cb)(void *cookie, dlna_service_t *service), void *cookie)
{
  int ret = 0;
  dlna_service_list_t *item;

  if (!dlna || !cb)
    return -1;

  for (item = dlna->services; item; item = item->hh.next)
  {
    ret = cb (cookie, item->service);
    if (ret)
      break;
  }
  return ret;
}

void
dlna_service_unregister (dlna_t *dlna, dlna_service_t *service)
{
  int id;
  dlna_service_list_t *item = NULL;

  id = service->typeid;
  HASH_FIND_INT (dlna->services, &id, item);

  dlna_service_free (dlna, item);
}

void
dlna_service_unregister_all (dlna_t *dlna)
{
  dlna_service_list_t *item;

  for (item = dlna->services; item; item = item->hh.next)
    dlna_service_free (dlna, item);
  dlna->services = NULL;
}
