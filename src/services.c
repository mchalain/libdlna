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
#include "cms.h"
#include "cds.h"
#include "avts.h"
#include "msr.h"

void
dlna_service_register (dlna_t *dlna, dlna_service_type_t srv)
{
  upnp_service_t *service;

  service = calloc (1, sizeof (upnp_service_t));
  
  switch (srv)
  {
  case DLNA_SERVICE_CONNECTION_MANAGER:
    service = &cms_service;
    break;
  case DLNA_SERVICE_CONTENT_DIRECTORY:
    service = &cds_service;
   break;
  case DLNA_SERVICE_AV_TRANSPORT:
    service = &avts_service;
    break;
  case DLNA_SERVICE_MS_REGISTAR:
    service = &msr_service;
    break;
  }

  HASH_ADD_STR (dlna->services, id, service);
}

static void
dlna_service_free (dlna_t *dlna, upnp_service_t *service)
{
  if (!dlna || !service)
    return;

  HASH_DEL (dlna->services, service);
}

upnp_service_t *
dlna_service_find (dlna_t *dlna, char *id)
{
  upnp_service_t *service;

  if (!dlna || !dlna->services || !id)
    return NULL;

  for (service = dlna->services; service; service = service->hh.next)
    if (service->id && id && !strcmp (service->id, id))
      return service;

  return NULL;
}

void
dlna_service_unregister (dlna_t *dlna, dlna_service_type_t srv)
{
  upnp_service_t *service;
  char *srv_id = NULL;
  
  switch (srv)
  {
  case DLNA_SERVICE_CONNECTION_MANAGER:
    srv_id = CMS_SERVICE_ID;
    break;
  case DLNA_SERVICE_CONTENT_DIRECTORY:
    srv_id = CDS_SERVICE_ID;
    break;
  case DLNA_SERVICE_AV_TRANSPORT:
    srv_id = AVTS_SERVICE_ID;
    break;
  case DLNA_SERVICE_MS_REGISTAR:
    srv_id = MSR_SERVICE_ID;
    break;
  }

  for (service = dlna->services; service; service = service->hh.next)
    if (service->id && srv_id && !strcmp (service->id, srv_id))
    {
      dlna_service_free (dlna, service);
      break;
    }
}

void
dlna_service_unregister_all (dlna_t *dlna)
{
  upnp_service_t *service;

  for (service = dlna->services; service; service = service->hh.next)
    dlna_service_free (dlna, service);
  dlna->services = NULL;
}
