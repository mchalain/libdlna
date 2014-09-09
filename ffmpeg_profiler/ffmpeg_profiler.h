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

#ifndef FFMPEG_PROFILER_H
#define FFMPEG_PROFILER_H

#include "dlna.h"

/***************************************************************************/
/*                                                                         */
/* DLNA Media Profiles Handling                                            */
/*  Mandatory: Used to register one or many DLNA profiles                  */
/*             you want your device to support.                            */
/*                                                                         */
/***************************************************************************/

typedef enum {
  /* Image Class */
  DLNA_PROFILE_IMAGE_JPEG,
  DLNA_PROFILE_IMAGE_PNG,
  /* Audio Class */
  DLNA_PROFILE_AUDIO_AC3,
  DLNA_PROFILE_AUDIO_AMR,
  DLNA_PROFILE_AUDIO_ATRAC3,
  DLNA_PROFILE_AUDIO_LPCM,
  DLNA_PROFILE_AUDIO_MP3,
  DLNA_PROFILE_AUDIO_MPEG4,
  DLNA_PROFILE_AUDIO_WMA,
  /* AV Class */
  DLNA_PROFILE_AV_MPEG1,
  DLNA_PROFILE_AV_MPEG2,
  DLNA_PROFILE_AV_MPEG4_PART2,
  DLNA_PROFILE_AV_MPEG4_PART10, /* a.k.a. MPEG-4 AVC */
  DLNA_PROFILE_AV_WMV9
} ffmpeg_profiler_media_profile_t;

/**
 * Register all known/supported DLNA profiles.
 *
 */
void ffmpeg_profiler_register_all_media_profiles ();

/**
 * Register one specific known/supported DLNA profiles.
 *
 * @param[in] profile  The profile ID to be registered.
 */
void
ffmpeg_profiler_register_media_profile (ffmpeg_profiler_media_profile_t profile);

extern const dlna_profiler_t ffmpeg_profiler;

#endif
