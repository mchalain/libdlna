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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dlna_internals.h"
#include "upnp_internals.h"
#include "dlna_db.h"
#include "cms.h"
#include "cds.h"
#include "avts.h"
#include "msr.h"
#include "rcs.h"

#define MEDIASERVER_VERSION 1
#define MEDIASERVER "MediaServer"

#define DLNA_DEVICE_DESCRIPTION_HEADER \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">" \
"  <specVersion>" \
"    <major>1</major>" \
"    <minor>0</minor>" \
"  </specVersion>" \
"  <device>" \
"    <deviceType>urn:schemas-upnp-org:device:%s:%d</deviceType>" \
"    <friendlyName>%s: 1</friendlyName>" \
"    <manufacturer>%s</manufacturer>" \
"    <manufacturerURL>%s</manufacturerURL>" \
"    <modelDescription>%s</modelDescription>" \
"    <modelName>%s</modelName>" \
"    <modelNumber>%s</modelNumber>" \
"    <modelURL>%s</modelURL>" \
"    <serialNumber>%s</serialNumber>" \
"    <UDN>uuid:%s</UDN>" \
"    <presentationURL>%s</presentationURL>"

#define DLNA_DLNADOC_DESCRIPTION \
"    <dlna:X_DLNADOC xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">DMS-1.00</dlna:X_DLNADOC>"

#define DLNA_DEVICE_DESCRIPTION_FOOTER \
"  </device>" \
"</root>"

#define DLNA_ICONLIST_HEADER \
"    <iconList>"

#define DLNA_ICONLIST_FOOTER \
"    </iconList>"

#define DLNA_SERVICELIST_HEADER \
"    <serviceList>"

#define DLNA_SERVICELIST_FOOTER \
"    </serviceList>"

#define DLNA_SERVICE_DESCRIPTION \
"      <service>" \
"        <serviceType>%s</serviceType>" \
"        <serviceId>%s</serviceId>" \
"        <SCPDURL>%s/%s</SCPDURL>" \
"        <controlURL>%s/%s</controlURL>" \
"        <eventSubURL>%s/%s</eventSubURL>" \
"      </service>" \

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
char *
dlna_dms_description_get (dlna_t *dlna)
{
  buffer_t *b = NULL;
  char *model_name, *desc = NULL;
  dlna_service_t *service;
  
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
  
  buffer_appendf (b, DLNA_DEVICE_DESCRIPTION_HEADER, MEDIASERVER, MEDIASERVER_VERSION,
                  dlna->friendly_name,
                  dlna->manufacturer, dlna->manufacturer_url,
                  dlna->model_description, model_name,
                  dlna->model_number, dlna->model_url,
                  dlna->serial_number, dlna->uuid,
                  dlna->presentation_url);

  free (model_name);

  if (dlna->mode & DLNA_CAPABILITY_DLNA)
    buffer_append (b, DLNA_DLNADOC_DESCRIPTION);
  if (dlna->services)
  {
    buffer_append (b, DLNA_SERVICELIST_HEADER);
    dlna_service_foreach (dlna, device_add_service, b);
    buffer_append (b, DLNA_SERVICELIST_FOOTER);
  }
  buffer_append (b, DLNA_DEVICE_DESCRIPTION_FOOTER);

  desc = strdup (b->buf);
  buffer_free (b);
  
  return desc;

}

int
dlna_dms_init (dlna_t *dlna)
{
  if (!dlna)
    return DLNA_ST_ERROR;

  if (!dlna->inited)
    return DLNA_ST_ERROR;

  dlna_service_register (dlna, &cms_service);
  dlna_service_register (dlna, &cds_service);
  dlna_service_register (dlna, &avts_service);
  if (dlna->mode == DLNA_CAPABILITY_UPNP_AV_XBOX)
    dlna_service_register (dlna, &msr_service);
  
  return upnp_init (dlna, DLNA_DEVICE_DMS);
}

int
dlna_dms_uninit (dlna_t *dlna)
{
  if (!dlna)
    return DLNA_ST_ERROR;

  if (!dlna->inited)
    return DLNA_ST_ERROR;

  dms_db_close (dlna);
  
  return upnp_uninit (dlna);
}

static void
dms_set_memory (dlna_t *dlna)
{
  if (!dlna)
    return;

  dlna->storage_type = DLNA_DMS_STORAGE_MEMORY;
  dlna_log (dlna, DLNA_MSG_INFO, "Use memory for VFS metadata storage.\n");
}

void
dlna_dms_set_vfs_storage_type (dlna_t *dlna,
                               dlna_dms_storage_type_t type, char *data)
{
  if (!dlna)
    return;

  if (type == DLNA_DMS_STORAGE_MEMORY)
    dms_set_memory (dlna);
  else if (type == DLNA_DMS_STORAGE_SQL_DB)
  {
    if (dms_db_open (dlna, data))
    {
      data = NULL;
      dms_set_memory (dlna);
      dlna_log (dlna, DLNA_MSG_WARNING,
              "SQLite support is disabled. " \
              "Failing back to Memory based VFS storage.\n");
      return;
    }
    else if (dms_db_check (dlna))
    {
      if (dms_db_create(dlna))
      {
        dms_set_memory (dlna);
        dlna_log (dlna, DLNA_MSG_WARNING,
              "SQLite support is disabled. " \
              "Failing back to Memory based VFS storage.\n");
        return;
      }
	}
  }
}
