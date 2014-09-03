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

#define AVTS_SERVICE_VERSION "1"
#define AVTS_SERVICE "AVTransport"
#define AVTS_SERVICE_ID   "urn:upnp-org:serviceId:"AVTS_SERVICE
#define AVTS_SERVICE_TYPE "urn:schemas-upnp-org:service:"AVTS_SERVICE":"AVTS_SERVICE_VERSION

#define AVTS_URL              "avts.xml"
#define AVTS_CONTROL_URL      "avts_control"
#define AVTS_EVENT_URL        "avts_event"

typedef struct dlna_dmp_item_s dlna_dmp_item_t;
struct dlna_dmp_item_s
{
  uint32_t id;
  dlna_item_t *item;
  dlna_dmp_item_t *current;
  UT_hash_handle hh;
};

extern dlna_service_t *avts_service_new (dlna_t*dlna);

#endif /* AVTS_H */
