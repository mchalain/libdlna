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

#define DLNA_DMS_DESCRIPTION_HEADER \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">" \
"  <specVersion>" \
"    <major>1</major>" \
"    <minor>0</minor>" \
"  </specVersion>" \
"  <device>" \
"    <deviceType>urn:schemas-upnp-org:device:MediaServer:1</deviceType>" \
"    <friendlyName>%s: 1</friendlyName>" \
"    <manufacturer>%s</manufacturer>" \
"    <manufacturerURL>%s</manufacturerURL>" \
"    <modelDescription>%s</modelDescription>" \
"    <modelName>%s</modelName>" \
"    <modelNumber>%s</modelNumber>" \
"    <modelURL>%s</modelURL>" \
"    <serialNumber>%s</serialNumber>" \
"    <UDN>uuid:%s</UDN>" \
"    <presentationURL>%s/%s</presentationURL>" \
"    <dlna:X_DLNADOC xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">DMS-1.00</dlna:X_DLNADOC>" \
"    <serviceList>"

#define DLNA_DMS_DESCRIPTION_FOOTER \
"    </serviceList>" \
"  </device>" \
"</root>"

#define DLNA_SERVICE_DESCRIPTION \
"      <service>" \
"        <serviceType>%s</serviceType>" \
"        <serviceId>%s</serviceId>" \
"        <SCPDURL>%s/%s</SCPDURL>" \
"        <controlURL>%s/%s</controlURL>" \
"        <eventSubURL>%s/%s</eventSubURL>" \
"      </service>" \

static void
dms_set_memory (dlna_t *dlna);
static void
dms_db_close(dlna_t *dlna);

char *
dlna_dms_description_get (dlna_t *dlna)
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
  
  buffer_appendf (b, DLNA_DMS_DESCRIPTION_HEADER, dlna->friendly_name,
                  dlna->manufacturer, dlna->manufacturer_url,
                  dlna->model_description, model_name,
                  dlna->model_number, dlna->model_url,
                  dlna->serial_number, dlna->uuid,
                  SERVICES_VIRTUAL_DIR, dlna->presentation_url);

  free (model_name);
  
  for (service = dlna->services; service; service = service->hh.next)
    buffer_appendf (b, DLNA_SERVICE_DESCRIPTION,
                    service->type, service->id,
                    SERVICES_VIRTUAL_DIR, service->scpd_url,
                    SERVICES_VIRTUAL_DIR, service->control_url,
                    SERVICES_VIRTUAL_DIR, service->event_url);

  buffer_append (b, DLNA_DMS_DESCRIPTION_FOOTER);

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

  dlna->dms.vfs_root = NULL;
  dlna->dms.vfs_items = 0;
#ifdef HAVE_SQLITE
  dlna->dms.db = NULL;
#endif /* HAVE_SQLITE */
  dms_set_memory(dlna);
  dlna_vfs_add_container (dlna, "root", 0, 0);

  dlna_service_register (dlna, DLNA_SERVICE_CONNECTION_MANAGER);
  dlna_service_register (dlna, DLNA_SERVICE_CONTENT_DIRECTORY);
  dlna_service_register (dlna, DLNA_SERVICE_AV_TRANSPORT);
  if (dlna->mode == DLNA_CAPABILITY_UPNP_AV_XBOX)
    dlna_service_register (dlna, DLNA_SERVICE_MS_REGISTAR);
  
  return upnp_init (dlna, DLNA_DEVICE_DMS);
}

int
dlna_dms_uninit (dlna_t *dlna)
{
  if (!dlna)
    return DLNA_ST_ERROR;

  if (!dlna->inited)
    return DLNA_ST_ERROR;

  vfs_item_free (dlna, dlna->dms.vfs_root);
  if (dlna->dms.storage_type == DLNA_DMS_STORAGE_DB)
    dms_db_close (dlna);
  return upnp_uninit (dlna);
}

static void
dms_set_memory (dlna_t *dlna)
{
  if (!dlna)
    return;

  dlna->dms.storage_type = DLNA_DMS_STORAGE_MEMORY;
  dlna_log (dlna, DLNA_MSG_INFO, "Use memory for VFS metadata storage.\n");
}

#ifdef HAVE_SQLITE
static int
dms_db_open (dlna_t *dlna, char *dbname)
{
  int res;

  if (!dlna)
    return -1;

  if (!dbname)
  {
    dlna_log (dlna, DLNA_MSG_ERROR,
              "SQLite support is disabled. " \
              "No database name has been provided");
    return -1;
  }

  res = sqlite3_open (dbname, &dlna->dms.db);
  if (res != SQLITE_OK)
  {
    dlna_log (dlna, DLNA_MSG_ERROR,
              "SQLite support is disabled. " \
              "Unable to open database '%s' (%s)",
              dbname, sqlite3_errmsg (dlna->dms.db));
    sqlite3_close (dlna->dms.db);
    return -1;
  }
  return 0;
}

static int
dms_db_load (dlna_t *dlna, vfs_item_t *root)
{
  return DLNA_ST_VFSEMPTY;
}

static void
dms_db_close (dlna_t *dlna)
{
  sqlite3_close (dlna->dms.db);
}
#else
static int
dms_db_open (dlna_t *dlna, char *dbname)
{
	return -1;
}

static int
dms_db_load (dlna_t *dlna, vfs_item_t *root)
{
  return DLNA_ST_VFSEMPTY;
}

static void
dms_db_close (dlna_t *dlna)
{
}
#endif /* HAVE_SQLITE */

int
dlna_dms_set_vfs_storage_type (dlna_t *dlna,
                               dlna_dms_storage_type_t type, char *data)
{
  int ret = DLNA_ST_VFSEMPTY;

  if (!dlna)
    return DLNA_ST_ERROR;

  if (type == DLNA_DMS_STORAGE_DB && !dms_db_open (dlna, data))
  {
    dlna->dms.storage_type = DLNA_DMS_STORAGE_DB;
    ret = dms_db_load (dlna, dlna->dms.vfs_root);
    dlna_log (dlna, DLNA_MSG_INFO,
            "Use SQL database for VFS metadata storage.\n");
  }
  else
	dms_set_memory (dlna);
  return ret;
}
