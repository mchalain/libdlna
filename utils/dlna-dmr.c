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
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <getopt.h>

#include "dlna.h"

static void
display_usage (char *name)
{
  printf ("Usage: %s [-u|d|x] [-c directory] [[-c directory]...]\n", name);
  printf ("Options:\n");
  printf (" -d\tStart in strict DLNA compliant mode\n");
  printf (" -h\tDisplay help\n");
  printf (" -u\tStart in pervasive UPnP A/V compliant mode\n");
}

int
main (int argc, char **argv)
{
  dlna_t *dlna;
  dlna_org_flags_t flags;
  dlna_capability_mode_t cap;
  int c, index;
  char short_options[] = "dhu";
  struct option long_options [] = {
    {"dlna", no_argument, 0, 'd' },
    {"help", no_argument, 0, 'h' },
    {"upnp", no_argument, 0, 'u' },
    {0, 0, 0, 0 }
  };

  printf ("libdlna Digital Media Renderer (DMR) API example\n");
  printf ("Using %s\n", LIBDLNA_IDENT);

  flags = DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE |
    DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE |
    DLNA_ORG_FLAG_CONNECTION_STALL |
    DLNA_ORG_FLAG_DLNA_V15;

  cap = DLNA_CAPABILITY_DLNA;

  /* command line argument processing */
  while (1)
  {
    c = getopt_long (argc, argv, short_options, long_options, &index);

    if (c == EOF)
      break;

    switch (c)
    {
    case 0:
      /* opt = long_options[index].name; */
      break;

    case 'h':
      display_usage (argv[0]);
      return -1;

    case 'd':
      cap = DLNA_CAPABILITY_DLNA;
      printf ("Running in strict DLNA compliant mode ...\n");
      break;

    case 'u':
      cap = DLNA_CAPABILITY_UPNP_AV;
      printf ("Running in pervasive UPnP A/V compliant mode ...\n");
      break;

    default:
      break;
    }
  }
  
  /* init DLNA stack */
  dlna = dlna_init ();
  dlna_set_org_flags (dlna, flags);
  dlna_set_verbosity (dlna, DLNA_MSG_INFO);
  dlna_set_capability_mode (dlna, cap);
  dlna_set_extension_check (dlna, 1);

  /* init Media profiler */

  /* define NIC to be used */
  dlna_set_interface (dlna, "eth0");

  /* set some UPnP device properties */
  dlna_device_set_friendly_name (dlna, "libdlna DMR template");
  dlna_device_set_uuid (dlna, "123456780");

  dlna_service_register (dlna, &cms_service);
  dlna_service_register (dlna, &rcs_service);
  dlna_service_register (dlna, &avts_service);

  /* initialize DMS: from this point you have a working/running media server */
  if (dlna_start (dlna, DLNA_DEVICE_DMR) != DLNA_ST_OK)
  {
    printf ("DMR init went wrong\n");
    dlna_uninit (dlna);
    return -1;
  }

  printf ("Hit 'q' or 'Q' + Enter to shutdown\n");
  while (1)
  {
    c = getchar ();
    if (c == 'q' || c == 'Q')
      break;
  }
  
  /* DMS shutdown */
  dlna_stop (dlna);

  /* DLNA stack cleanup */
  dlna_uninit (dlna);
  
  return 0;
}
