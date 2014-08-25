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

#ifndef AVTS_H
#define AVTS_H

#define AVTS_LOCATION "/services/avts.xml"

#define AVTS_SERVICE_VERSION "1"
#define AVTS_SERVICE_ID   "urn:upnp-org:serviceId:AVTransport"
#define AVTS_SERVICE_TYPE "urn:schemas-upnp-org:service:AVTransport:"AVTS_SERVICE_VERSION

#define AVTS_URL              "avts.xml"
#define AVTS_CONTROL_URL      "avts_control"
#define AVTS_EVENT_URL        "avts_event"

extern upnp_service_t avts_service;

#endif /* AVTS_H */
