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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <dlfcn.h>

#if (defined(BSD) || defined(__FreeBSD__) || defined(__APPLE__))
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if_dl.h>
#endif

#if (defined(__APPLE__))
#include <net/route.h>
#endif

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>

#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif

#include "dlna_internals.h"
#include "upnp_internals.h"
#include "devices.h"
#include "vfs.h"

dlna_verbosity_level_t dlna_verbosity = DLNA_MSG_ERROR;
dlna_capability_mode_t dlna_mode;

void
dlna_add_profiler_library (dlna_t *dlna, char *librarypath)
{
  void *handle;
  handle = dlopen (librarypath, RTLD_LAZY);
  if (handle)
  {
    char *error;
    const dlna_profiler_t **profiler;

    dlerror ();
    profiler = dlsym (handle, "profiler");
    error = dlerror ();
    if (error)
      dlna_log (DLNA_MSG_ERROR, "error %s\n", error);
    if (profiler)
      dlna_add_profiler (dlna, *profiler);
  }
}

void
dlna_add_profiler (dlna_t *dlna, const dlna_profiler_t *profiler)
{
  if (!dlna)
    return;
  
  if (!dlna->profilers)
  {
    dlna->profilers = calloc (1, sizeof (dlna_profiler_list_t));
    dlna->profilers->profiler = profiler;
    dlna->profilers->next = NULL;
  }
  else
  {
    dlna_profiler_list_t *profilers_list;

    profilers_list = calloc (1, sizeof (dlna_profiler_list_t));
    profilers_list->profiler = profiler;
    profilers_list->next = dlna->profilers;
    dlna->profilers = profilers_list;
  }

  /* use internal function to initialize the profiler */
  if (profiler->init)
	profiler->init (dlna);
}

dlna_t *
dlna_init (void)
{
  dlna_t *dlna;

  dlna = calloc (1, sizeof (dlna_t));
  dlna->inited = 1;
  dlna->mode = 0;
  dlna_mode = 0;
  dlna->check_extensions = 1;

  dlna->interface = strdup ("lo"); /* bind to loopback as a default */
  dlna->port = 0;
  
  dlna_log (DLNA_MSG_INFO, "DLNA: init\n");

  return dlna;
}

static void
dlna_remove_profilers (dlna_t *dlna)
{
  if (!dlna)
    return;
  
  while (dlna->profilers)
  {
    dlna_profiler_list_t *profilers_list = dlna->profilers->next;

    if (dlna->profilers->profiler->free)
      dlna->profilers->profiler->free (dlna->profilers->profiler);
    free (dlna->profilers);
    dlna->profilers = profilers_list;
  }
}

void
dlna_uninit (dlna_t *dlna)
{
  if (!dlna)
    return;

  dlna->inited = 0;
  dlna_log (DLNA_MSG_INFO, "DLNA: uninit\n");
  free (dlna->interface);

  dlna_remove_profilers (dlna);
  /* UPnP Properties */
  if (dlna->device)
    dlna_device_free (dlna->device);

  free (dlna);
}

void
dlna_log (dlna_verbosity_level_t level, const char *format, ...)
{
  va_list va;

  if (!format)
    return;

  /* do we really want loging ? */
  if (dlna_verbosity == DLNA_MSG_NONE)
    return;
  
  if (level < dlna_verbosity)
    return;

  va_start (va, format);
  fprintf (stderr, "[libdlna] ");
  vfprintf (stderr, format, va);
  va_end (va);
}

void
dlna_set_device (dlna_t *dlna, struct dlna_device_s *device)
{
  if (dlna->device)
    dlna_device_free (dlna->device);
  dlna->device = device;
}

void
dlna_set_verbosity (dlna_t *dlna, dlna_verbosity_level_t level)
{
  if (!dlna)
    return;

  dlna_verbosity = level;
}

void
dlna_set_capability_mode (dlna_t *dlna, dlna_capability_mode_t mode)
{
  if (!dlna)
    return;

  dlna->mode = mode;
  dlna_mode = mode;

  if (dlna->mode & DLNA_CAPABILITY_DLNA)
    dlna->check_extensions = 1;
  if (dlna->mode & DLNA_CAPABILITY_UPNP_AV)
    dlna_add_profiler (dlna, &upnpav_profiler);
}

void
dlna_set_extension_check (dlna_t *dlna, int level)
{
  if (!dlna)
    return;

  if (dlna->mode & DLNA_CAPABILITY_DLNA)
    return;
  
  dlna->check_extensions = level;
}

static int
has_network_interface (char *interface)
{
#ifdef HAVE_IFADDRS_H
  struct ifaddrs *itflist, *itf;

  if (!interface)
    return 0;

  if (getifaddrs (&itflist) < 0)
  {
    perror ("getifaddrs");
    return 0;
  }

  itf = itflist;
  while (itf)
  {
    if ((itf->ifa_flags & IFF_UP)
        && !strncmp (itf->ifa_name, interface, IFNAMSIZ))
    {
      freeifaddrs (itflist);
      return 1;
    }
    itf = itf->ifa_next;
  }

  freeifaddrs (itflist);
#else  
  int sock, i, n;
  struct ifconf ifc;
  struct ifreq ifr;
  char buff[8192];

  if (!interface)
    return 0;

  /* determine UDN according to MAC address */
  sock = socket (AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror ("socket");
    return 0;
  }

  /* get list of available interfaces */
  ifc.ifc_len = sizeof (buff);
  ifc.ifc_buf = buff;

  if (ioctl (sock, SIOCGIFCONF, &ifc) < 0)
  {
    perror ("ioctl");
    close (sock);
    return 0;
  }

  n = ifc.ifc_len / sizeof (struct ifreq);
  for (i = n - 1 ; i >= 0 ; i--)
  {
    ifr = ifc.ifc_req[i];

    if (strncmp (ifr.ifr_name, interface, IFNAMSIZ))
      continue;

    if (ioctl (sock, SIOCGIFFLAGS, &ifr) < 0)
    {
      perror ("ioctl");
      close (sock);
      return 0;
    }

    if (!(ifr.ifr_flags & IFF_UP))
    {
      close (sock);
      return 0;
    }

    /* found right interface */
    close (sock);
    return 1;
  }
  close (sock);
#endif

  return 0;
}

void
dlna_set_interface (dlna_t *dlna, char *itf)
{
  if (!dlna || !itf)
    return;

  /* check for valid network interface */
  if (!has_network_interface (itf))
    return;
  
  if (dlna->interface)
    free (dlna->interface);
  dlna->interface = strdup (itf);
}

void
dlna_set_port (dlna_t *dlna, int port)
{
  if (!dlna)
    return;

  /* check for valid port number */
  if (port <= 0 || port > 65535)
    return;
  
  dlna->port = port;
}
