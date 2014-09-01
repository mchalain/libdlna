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
#include <stdio.h>

#include "dlna_internals.h"
#include "upnp_internals.h"
#include "buffer.h"
#include "devices.h"

void
dlna_device_set_type (dlna_t *dlna, char *str, char *short_str)
{
  if (!dlna || !str)
    return;

  if (dlna->urn_type)
    free (dlna->urn_type);
  dlna->urn_type = strdup (str);
}


void
dlna_device_set_friendly_name (dlna_t *dlna, char *str)
{
  if (!dlna || !str)
    return;

  if (dlna->friendly_name)
    free (dlna->friendly_name);
  dlna->friendly_name = strdup (str);
}

void
dlna_device_set_manufacturer (dlna_t *dlna, char *str)
{
  if (!dlna || !str)
    return;

  if (dlna->manufacturer)
    free (dlna->manufacturer);
  dlna->manufacturer = strdup (str);
}

void
dlna_device_set_manufacturer_url (dlna_t *dlna, char *str)
{
  if (!dlna || !str)
    return;

  if (dlna->manufacturer_url)
    free (dlna->manufacturer_url);
  dlna->manufacturer_url = strdup (str);
}

void
dlna_device_set_model_description (dlna_t *dlna, char *str)
{
  if (!dlna || !str)
    return;

  if (dlna->model_description)
    free (dlna->model_description);
  dlna->model_description = strdup (str);
}

void
dlna_device_set_model_name (dlna_t *dlna, char *str)
{
  if (!dlna || !str)
    return;

  if (dlna->model_name)
    free (dlna->model_name);
  dlna->model_name = strdup (str);
}

void
dlna_device_set_model_number (dlna_t *dlna, char *str)
{
  if (!dlna || !str)
    return;

  if (dlna->model_number)
    free (dlna->model_number);
  dlna->model_number = strdup (str);
}

void
dlna_device_set_model_url (dlna_t *dlna, char *str)
{
  if (!dlna || !str)
    return;

  if (dlna->model_url)
    free (dlna->model_url);
  dlna->model_url = strdup (str);
}


void
dlna_device_set_serial_number (dlna_t *dlna, char *str)
{
  if (!dlna || !str)
    return;

  if (dlna->serial_number)
    free (dlna->serial_number);
  dlna->serial_number = strdup (str);
}

void
dlna_device_set_uuid (dlna_t *dlna, char *str)
{
  if (!dlna || !str)
    return;

  if (dlna->uuid)
    free (dlna->uuid);
  dlna->uuid = strdup (str);
}

void
dlna_device_set_presentation_url (dlna_t *dlna, char *str)
{
  if (!dlna || !str)
    return;

  if (dlna->presentation_url)
    free (dlna->presentation_url);
  dlna->presentation_url = strdup (str);
}

char *
dlna_device_get_description (dlna_t *dlna)
{
  buffer_t *b = NULL;
  char *model_name, *desc = NULL;
  upnp_service_t *service;
  
  if (!dlna)
    return NULL;

  if (dlna->mode == DLNA_CAPABILITY_UPNP_AV_XBOX)
  {
    model_name =
      malloc (strlen (XBOX_MODEL_NAME) + strlen (dlna->model_name) + 4);
    sprintf (model_name, "%s (%s)", XBOX_MODEL_NAME, dlna->model_name);
  }
  else
    model_name = strdup (dlna->model_name);

  b = buffer_new ();
  
  buffer_appendf (b, DLNA_DESCRIPTION_HEADER, 
                  dlna->urn_type, dlna->friendly_name,
                  dlna->manufacturer, dlna->manufacturer_url,
                  dlna->model_description, model_name,
                  dlna->model_number, dlna->model_url,
                  dlna->serial_number, dlna->uuid);

  free (model_name);
  
  if (dlna->presentation_url)
    buffer_appendf (b, DLNA_DEVICE_PRESENTATION, dlna->presentation_url);

  if (dlna->mode & DLNA_CAPABILITY_DLNA)
    buffer_append (b, DLNA_DLNADOC_DESCRIPTION);

  if (dlna->services)
  {
    buffer_appendf (b, DLNA_SERVICELIST_HEADER);
    for (service = dlna->services; service; service = service->hh.next)
      buffer_appendf (b, DLNA_SERVICE_DESCRIPTION,
                      service->type, service->id,
                      SERVICES_VIRTUAL_DIR, service->scpd_url,
                      SERVICES_VIRTUAL_DIR, service->control_url,
                      SERVICES_VIRTUAL_DIR, service->event_url);
    buffer_appendf (b, DLNA_SERVICELIST_FOOTER);
  }

  buffer_append (b, DLNA_DESCRIPTION_FOOTER);

  desc = strdup (b->buf);
  buffer_free (b);
  
  return desc;

}

