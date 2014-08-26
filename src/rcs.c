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

/*
 * AVTransport service specifications can be found at:
 * http://upnp.org/standardizeddcps/documents/AVTransport1.0.pdf
 * http://www.upnp.org/specs/av/UPnP-av-AVTransport-v2-Service-20060531.pdf
 */

#include "upnp_internals.h"
#include "rcs.h"
    
/* List of UPnP Rendering Control Service actions */
upnp_service_action_t RCS_service_actions[] = {
  { RCS_ACTION_LIST_PRESETS,           NULL },
  { RCS_ACTION_SELECT_PRESET,      NULL },
  { RCS_ACTION_GET_BRIGHTNESS,    NULL },
  { RCS_ACTION_SET_BRIGHTNESS,          NULL },
  { RCS_ACTION_GET_CONTRAST,      NULL },
  { RCS_ACTION_SET_CONTRAST,          NULL },
  { RCS_ACTION_GET_SHARPNESS,      NULL },
  { RCS_ACTION_SET_SHARPNESS,              NULL },
  { RCS_ACTION_GET_R_V_GAIN,              NULL },
  { RCS_ACTION_SET_R_V_GAIN,             NULL },
  { RCS_ACTION_GET_G_V_GAIN,            NULL },
  { RCS_ACTION_SET_G_V_GAIN,              NULL },
  { RCS_ACTION_GET_B_V_GAIN,              NULL },
  { RCS_ACTION_SET_B_V_GAIN,          NULL },
  { RCS_ACTION_GET_R_V_BLEVEL,     NULL },
  { RCS_ACTION_SET_R_V_BLEVEL,   NULL },
  { RCS_ACTION_GET_G_V_BLEVEL,       NULL },
  { RCS_ACTION_SET_G_V_BLEVEL,       NULL },
  { RCS_ACTION_GET_B_V_BLEVEL,       NULL },
  { RCS_ACTION_SET_B_V_BLEVEL,       NULL },
  { NULL,                                  NULL }
};

char *
rcs_get_description()
{
  return strdup(RCS_DESCRIPTION);
}
