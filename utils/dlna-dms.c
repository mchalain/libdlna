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
#include "http_protocol.h"

static void
add_dir (dlna_t *dlna, dlna_vfs_t *vfs, char *dir, uint32_t id)
{
  struct dirent **namelist;
  int n, i;

  n = scandir (dir, &namelist, 0, alphasort);
  if (n < 0)
  {
    perror ("scandir");
    return;
  }

  for (i = 0; i < n; i++)
  {
    struct stat st;
    char *fullpath = NULL;

    if (namelist[i]->d_name[0] == '.')
    {
      free (namelist[i]);
      continue;
    }

    fullpath = malloc (strlen (dir) + strlen (namelist[i]->d_name) + 2);
    sprintf (fullpath, "%s/%s", dir, namelist[i]->d_name);

    if (stat (fullpath, &st) < 0)
    {
      free (namelist[i]);
      free (fullpath);
      continue;
    }

    if (S_ISDIR (st.st_mode))
    {
      uint32_t cid;
      cid = dlna_vfs_add_container (vfs, fullpath, 0, id);
      add_dir (dlna, vfs, fullpath, cid);
    }
    else
    {
      dlna_item_t *item;
      item = dlna_item_new (dlna, fullpath);
      if (item)
      {
        dlna_vfs_add_resource (vfs, basename (fullpath),
                             item, id);
      }
    }
    
    free (namelist[i]);
    free (fullpath);
  }
  free (namelist);
}

static void
display_usage (char *name)
{
  printf ("Usage: %s [-u|d|x] [-c directory] [[-c directory]...]\n", name);
  printf ("Options:\n");
  printf (" -c\tContent directory to be shared\n");
  printf (" -i\tNetwork interface\n");
  printf (" -d\tStart in strict DLNA compliant mode\n");
  printf (" -h\tDisplay help\n");
  printf (" -u\tStart in pervasive UPnP A/V compliant mode\n");
  printf (" -x\tStart in hackish XboX 360 UPnP A/V compliant mode\n");
}

int
main (int argc, char **argv)
{
  dlna_t *dlna;
  dlna_device_t *device;
  dlna_capability_mode_t cap = 0;
  dlna_vfs_t *vfs;
  char *interface = NULL;
  int c, index;
  char *content_dir = NULL;
  struct stat st;
  char short_options[] = "c:i:dhux";
  struct option long_options [] = {
    {"content", required_argument, 0, 'c' },
    {"interface", required_argument, 0, 'i' },
    {"dlna", no_argument, 0, 'd' },
    {"help", no_argument, 0, 'h' },
    {"upnp", no_argument, 0, 'u' },
    {"xbox", no_argument, 0, 'x' },
    {0, 0, 0, 0 }
  };

  printf ("libdlna Digital Media Server (DMS) API example\n");
  printf ("Using %s\n", LIBDLNA_IDENT);

  if (argc == 1)
  {
    display_usage (argv[0]);
    return -1;
  }
 
  cap = 0;

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

    case 'c':
      content_dir = strdup (optarg);
      break;

    case 'i':
      interface = strdup (optarg);
      break;

    case 'd':
      cap |= DLNA_CAPABILITY_DLNA;
      printf ("Running in strict DLNA compliant mode ...\n");
      break;

    case 'u':
      cap |= DLNA_CAPABILITY_UPNP_AV;
      printf ("Running in pervasive UPnP A/V compliant mode ...\n");
      break;

    case 'x':
      cap |= DLNA_CAPABILITY_UPNP_AV_XBOX;
      printf ("Running in hackish XboX 360 UPnP A/V compliant mode ...\n");
      break;

    default:
      break;
    }
  }
  
  if (!interface)
  {
    interface = strdup ("eth0");
  }
  if (!content_dir)
  {
    printf ("No content directory to be shared, bail out.\n");
    return -1;
  }
  
  /* init DLNA stack */
  dlna = dlna_init ();
  dlna_set_verbosity (dlna, DLNA_MSG_INFO);
  dlna_set_capability_mode (dlna, cap);
  dlna_set_extension_check (dlna, 1);

  /* init Media profiler */
  if ((cap & DLNA_CAPABILITY_DLNA) || !(cap & DLNA_CAPABILITY_UPNP_AV))
  {
    dlna_add_profiler_library (dlna, "./ffmpeg_profiler/libdlnaprofiler_ffmpeg.so");
    dlna_add_profiler_library (dlna, "./mpg123_profiler/libdlnaprofiler_mpg123.so");
  }

  /* define NIC to be used */
  dlna_set_interface (dlna, interface);

  /* set some UPnP device properties */
  device = dlna_device_new (cap);
  dlna_device_set_type (device, DLNA_DEVICE_TYPE_DMS,"DMS");
  dlna_device_set_friendly_name (device, "libdlna DMS template");
  dlna_device_set_uuid (device, "123456789");

  dlna_service_register (device, cms_service_new(dlna));
  vfs = dlna_vfs_new (dlna);
  dlna_vfs_add_protocol (vfs, http_protocol_new(dlna));
  dlna_service_register (device, cds_service_new(dlna, vfs));
  if (cap & DLNA_CAPABILITY_UPNP_AV_XBOX)
    dlna_service_register (device, msr_service_new(dlna));
  
  dlna_set_device (dlna, device);

  printf ("Trying to share '%s'\n", content_dir);
  if (stat (content_dir, &st) < 0)
  {
    printf ("Invalid content directory\n");
    return -1;
  }
  if (S_ISDIR (st.st_mode))
    add_dir (dlna, vfs, content_dir, 0);
  else
  {
    dlna_item_t *item;

    item = dlna_item_new (dlna, content_dir);
    if (item)
    {
      dlna_vfs_add_resource (vfs, basename (content_dir),
                           item, 0);
    }
  }
  
  if (dlna_start (dlna) != DLNA_ST_OK)
  {
    printf ("DMS init went wrong\n");
    dlna_uninit (dlna);
    dlna_vfs_free (vfs);
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
  
  dlna_vfs_free (vfs);

  free (interface);
  free (content_dir);
  
  return 0;
}
