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

#include "dlna.h"
#include "dlna_internals.h"
#include "ffmpeg_profiler/ffmpeg_profiler.h"

int
main (int argc, char **argv)
{
  dlna_t *dlna;
  dlna_profile_t *p;
  dlna_org_flags_t flags;
  dlna_item_t *item;
  void *cookie;
  
  if (argc < 2)
  {
    printf ("usage: %s media_filename\n", argv[0]);
    return -1;
  }

  flags = DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE |
    DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE |
    DLNA_ORG_FLAG_CONNECTION_STALL |
    DLNA_ORG_FLAG_DLNA_V15;
  
  printf ("Using %s\n", LIBDLNA_IDENT);
  
  dlna = dlna_init ();
  dlna_set_org_flags (dlna, flags);
  dlna_set_verbosity (dlna, DLNA_MSG_INFO);
  ffmpeg_profiler_register_all_media_profiles ();

  item = dlna_item_new (dlna, argv[1]);
  if (item)
  {
    if (item->properties)
    {
      printf ("Size: %lld bytes\n", item->filesize);
      printf ("Duration: %s\n", item->properties->duration);
      printf ("Bitrate: %d bytes/sec\n", item->properties->bitrate);
      printf ("SampleFrequency: %d Hz\n", item->properties->sample_frequency);
      printf ("BitsPerSample: %d\n", item->properties->bps);
      printf ("Channels: %d\n", item->properties->channels);
      printf ("Resolution: %s\n", item->properties->resolution);
    }

    if (item->metadata)
    {
      printf ("Title: %s\n", item->metadata->title);
      printf ("Artist: %s\n", item->metadata->author);
      printf ("Description: %s\n", item->metadata->comment);
      printf ("Album: %s\n", item->metadata->album);
      printf ("Track: %d\n", item->metadata->track);
      printf ("Genre: %s\n", item->metadata->genre);
    }
    dlna_item_free (item);
  }
  
  p = ffmpeg_profiler_guess_media_profile (argv[1], &cookie);
  if (p)
  {
    char *protocol_info;
    
    printf ("ID: %s\n", p->id);
    printf ("MIME: %s\n", p->mime);
    printf ("Label: %s\n", p->label);
    printf ("Class: %d\n", p->media_class);
    printf ("UPnP Object Item: %s\n", dlna_profile_upnp_object_item (p));

    protocol_info = dlna_write_protocol_info (dlna, DLNA_PROTOCOL_INFO_TYPE_HTTP,
                                              DLNA_ORG_PLAY_SPEED_NORMAL,
                                              DLNA_ORG_CONVERSION_NONE,
                                              DLNA_ORG_OPERATION_RANGE,
                                              flags, p);
    printf ("Protocol Info: %s\n", protocol_info);
    free (protocol_info);
  }
  else
    printf ("Unknown format\n");

  dlna_uninit (dlna);
  
  return 0;
}
