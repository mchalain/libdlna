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
#include "services.h"

static char *
dlna_device_get_description (dlna_t *dlna);

dlna_device_t *
dlna_device_new ()
{
  dlna_device_t *device;

  device = calloc (1, sizeof (dlna_device_t));
  
  device->services = NULL;

  device->friendly_name = strdup ("libdlna");
  device->manufacturer = strdup ("Benjamin Zores");
  device->manufacturer_url = strdup ("http://libdlna.geexbox.org/");
  device->model_description = strdup ("libdlna device");
  device->model_name = strdup ("libdlna");
  device->model_number = strdup ("libdlna-001");
  device->model_url = strdup ("http://libdlna.geexbox.org/");
  device->serial_number = strdup ("libdlna-001");
  device->uuid = strdup ("01:23:45:67:89");
  device->presentation_url = strdup (SERVICES_VIRTUAL_DIR "/presentation.html");

  device->get_description = dlna_device_get_description;
  return device;
}

void
dlna_device_free (dlna_device_t *device)
{
  dlna_service_unregister_all (device);
  
  free (device->friendly_name);
  free (device->manufacturer);
  free (device->manufacturer_url);
  free (device->model_description);
  free (device->model_name);
  free (device->model_number);
  free (device->model_url);
  free (device->serial_number);
  free (device->uuid);
  free (device->presentation_url);
}

void
dlna_device_set_type (dlna_device_t *device, char *str, char *short_str)
{
  if (!device || !str)
    return;

  if (device->urn_type)
    free (device->urn_type);
  device->urn_type = strdup (str);
}


void
dlna_device_set_friendly_name (dlna_device_t *device, char *str)
{
  if (!device || !str)
    return;

  if (device->friendly_name)
    free (device->friendly_name);
  device->friendly_name = strdup (str);
}

void
dlna_device_set_manufacturer (dlna_device_t *device, char *str)
{
  if (!device || !str)
    return;

  if (device->manufacturer)
    free (device->manufacturer);
  device->manufacturer = strdup (str);
}

void
dlna_device_set_manufacturer_url (dlna_device_t *device, char *str)
{
  if (!device || !str)
    return;

  if (device->manufacturer_url)
    free (device->manufacturer_url);
  device->manufacturer_url = strdup (str);
}

void
dlna_device_set_model_description (dlna_device_t *device, char *str)
{
  if (!device || !str)
    return;

  if (device->model_description)
    free (device->model_description);
  device->model_description = strdup (str);
}

void
dlna_device_set_model_name (dlna_device_t *device, char *str)
{
  if (!device || !str)
    return;

  if (device->model_name)
    free (device->model_name);
  device->model_name = strdup (str);
}

void
dlna_device_set_model_number (dlna_device_t *device, char *str)
{
  if (!device || !str)
    return;

  if (device->model_number)
    free (device->model_number);
  device->model_number = strdup (str);
}

void
dlna_device_set_model_url (dlna_device_t *device, char *str)
{
  if (!device || !str)
    return;

  if (device->model_url)
    free (device->model_url);
  device->model_url = strdup (str);
}


void
dlna_device_set_serial_number (dlna_device_t *device, char *str)
{
  if (!device || !str)
    return;

  if (device->serial_number)
    free (device->serial_number);
  device->serial_number = strdup (str);
}

void
dlna_device_set_uuid (dlna_device_t *device, char *str)
{
  if (!device || !str)
    return;

  if (device->uuid)
    free (device->uuid);
  device->uuid = strdup (str);
}

void
dlna_device_set_presentation_url (dlna_device_t *device, char *str)
{
  if (!device || !str)
    return;

  if (device->presentation_url)
    free (device->presentation_url);
  device->presentation_url = strdup (str);
}

static int
device_add_service (void *cookie, dlna_service_t *service)
{
  buffer_t *b = cookie;

  buffer_appendf (b, DLNA_SERVICE_DESCRIPTION,
                service->type, service->id,
                SERVICES_VIRTUAL_DIR, service->scpd_url,
                SERVICES_VIRTUAL_DIR, service->control_url,
                SERVICES_VIRTUAL_DIR, service->event_url);

  return 0;
}

static char *
dlna_device_get_description (dlna_t *dlna)
{
  buffer_t *b = NULL;
  char *model_name, *desc = NULL;
  dlna_device_t *device;
  
  if (!dlna || !dlna->device)
    return NULL;

  device = dlna->device;

  if (dlna->mode == DLNA_CAPABILITY_UPNP_AV_XBOX)
  {
    model_name =
      malloc (strlen (XBOX_MODEL_NAME) + strlen (device->model_name) + 4);
    sprintf (model_name, "%s (%s)", XBOX_MODEL_NAME, device->model_name);
  }
  else
    model_name = strdup (device->model_name);

  b = buffer_new ();
  
  buffer_appendf (b, DLNA_DESCRIPTION_HEADER, 
                  device->urn_type, device->friendly_name,
                  device->manufacturer, device->manufacturer_url,
                  device->model_description, model_name,
                  device->model_number, device->model_url,
                  device->serial_number, device->uuid);

  free (model_name);
  
  if (device->presentation_url)
    buffer_appendf (b, DLNA_DEVICE_PRESENTATION, device->presentation_url);

  if (dlna->mode & DLNA_CAPABILITY_DLNA)
    buffer_append (b, DLNA_DLNADOC_DMS_DESCRIPTION);

  if (device->services)
  {
    buffer_appendf (b, DLNA_SERVICELIST_HEADER);
    dlna_service_foreach (device, device_add_service, b);
    buffer_appendf (b, DLNA_SERVICELIST_FOOTER);
  }

  buffer_append (b, DLNA_DESCRIPTION_FOOTER);

  desc = strdup (b->buf);
  buffer_free (b);
  
  return desc;

}

