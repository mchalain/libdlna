/*
 * libdlna: reference DLNA standards implementation.
 * Copyright (C) 2014-2016 Marc Chalain <marc.chalain@gmail.com>
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

#ifndef DEVICES_H
#define DEVICES_H

typedef struct dlna_device_s dlna_device_t;

struct dlna_device_s {
  /* UPnP Services */
  struct dlna_service_list_s *services;
  dlna_capability_mode_t mode;

  char *(*get_description) (dlna_t *);

  char *urn_type;
  char *friendly_name;
  char *manufacturer;
  char *manufacturer_url;
  char *model_description;
  char *model_name;
  char *model_number;
  char *model_url;
  char *serial_number;
  char *uuid;
  char *presentation_url;
  char *dlnadoc;
  char *dlnacap;
};

dlna_device_t *dlna_device_new (dlna_capability_mode_t mode);
void dlna_device_free (dlna_device_t *device);

#define DLNA_DESCRIPTION_HEADER \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">" \
"  <specVersion>" \
"    <major>1</major>" \
"    <minor>0</minor>" \
"  </specVersion>" \
"  <device>" \
"    <deviceType>%s</deviceType>" \
"    <friendlyName>%s: 1</friendlyName>" \
"    <manufacturer>%s</manufacturer>" \
"    <manufacturerURL>%s</manufacturerURL>" \
"    <modelDescription>%s</modelDescription>" \
"    <modelName>%s</modelName>" \
"    <modelNumber>%s</modelNumber>" \
"    <modelURL>%s</modelURL>" \
"    <serialNumber>%s</serialNumber>" \
"    <UDN>uuid:%s</UDN>"

#define DLNA_DEVICE_PRESENTATION \
"    <presentationURL>%s</presentationURL>"

#define DLNA_DLNADOC_DESCRIPTION \
"    <dlna:X_DLNADOC xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">%s-1.00</dlna:X_DLNADOC>"

#define DLNA_DLNADOC_M_DESCRIPTION \
"    <dlna:X_DLNADOC xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">M-%s-1.00</dlna:X_DLNADOC>"

#define DLNA_ICONLIST_HEADER \
"    <iconList>"

#define DLNA_ICON_DESCRIPTION \
"    <icon>" \
"      <mimetype>%s</mimetype>" \
"      <width>%d</width>" \
"      <height>%d</height>" \
"      <depth>%d</depth>" \
"      <url>%s</url>" \
"    </icon>"

#define DLNA_ICONLIST_FOOTER \
"    </iconList>"

#define DLNA_SERVICELIST_HEADER \
"    <serviceList>"

#define DLNA_SERVICELIST_FOOTER \
"    </serviceList>"

#define DLNA_DESCRIPTION_FOOTER \
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

#endif
