/*
 * libdlna: reference DLNA standards implementation.
 * Copyright (C) 2007-2008 Benjamin Zores <ben@geexbox.org>
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

#ifndef CDS_H
#define CDS_H

#define CDS_SERVICE_ID   "urn:upnp-org:serviceId:ContentDirectory"
#define CDS_SERVICE_TYPE "urn:schemas-upnp-org:service:ContentDirectory:2"

#define CDS_URL              "cds.xml"
#define CDS_CONTROL_URL      "cds_control"
#define CDS_EVENT_URL        "cds_event"

extern dlna_service_t *cds_service_new (dlna_t*dlna, struct dlna_vfs_s *vfs);
void cds_vfs_changed (dlna_service_t *service);

#endif /* CDS_H */
