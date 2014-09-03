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

#ifndef MSR_H
#define MSR_H

#define MSR_SERVICE_ID "urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar"
#define MSR_SERVICE_TYPE "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1"

#define MSR_URL              "msr.xml"
#define MSR_CONTROL_URL      "msr_control"
#define MSR_EVENT_URL        "msr_event"

extern dlna_service_t *msr_service_new (dlna_t*dlna);

#endif /* MSR_H */
