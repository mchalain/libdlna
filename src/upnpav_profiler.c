
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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "dlna_internals.h"

static const dlna_profile_t *profiles_list[] = {
  /* Video files */
  &(dlna_profile_t) { .ext = ".asf", .mime = MIME_VIDEO_ASF, .media_class = DLNA_CLASS_AV,.id = "WMVMED_BASE",},
  &(dlna_profile_t) { .ext = ".avc", .mime = MIME_VIDEO_AVI, .media_class = DLNA_CLASS_AV,.id = "AVC_MP4_MP_SD_AC3",},
  &(dlna_profile_t) { .ext = ".avi", .mime = MIME_VIDEO_AVI, .media_class = DLNA_CLASS_AV,},
  &(dlna_profile_t) { .ext = ".dv", .mime = "video/x-dv", .media_class = DLNA_CLASS_AV,},
  &(dlna_profile_t) { .ext = ".divx", .mime = MIME_VIDEO_AVI, .media_class = DLNA_CLASS_AV,.id = "MPEG4_P2_MP4_SP_AAC",},
  &(dlna_profile_t) { .ext = ".wmv", .mime = MIME_VIDEO_WMV, .media_class = DLNA_CLASS_AV,.id = "WMVMED_BASE",},
  &(dlna_profile_t) { .ext = ".mjpg", .mime = "video/x-motion-jpeg", .media_class = DLNA_CLASS_AV,},
  &(dlna_profile_t) { .ext = ".mjpeg", .mime = "video/x-motion-jpeg", .media_class = DLNA_CLASS_AV,},
  &(dlna_profile_t) { .ext = ".mpeg", .mime = MIME_VIDEO_MPEG, .media_class = DLNA_CLASS_AV,.id = "MPEG1",},
  &(dlna_profile_t) { .ext = ".mpg", .mime = MIME_VIDEO_MPEG, .media_class = DLNA_CLASS_AV,.id = "MPEG1",},
  &(dlna_profile_t) { .ext = ".mpe", .mime = MIME_VIDEO_MPEG, .media_class = DLNA_CLASS_AV,.id = "MPEG1",},
  &(dlna_profile_t) { .ext = ".mp2p", .mime = "video/mp2p", .media_class = DLNA_CLASS_AV,.id = "MPEG_PS_PAL",},
  &(dlna_profile_t) { .ext = ".vob", .mime = "video/mp2p", .media_class = DLNA_CLASS_AV,.id = "MPEG_PS_PAL",},
  &(dlna_profile_t) { .ext = ".mp2t", .mime = "video/mp2t", .media_class = DLNA_CLASS_AV,.id = "MPEG_TS_SD_EU",},
  &(dlna_profile_t) { .ext = ".m1v", .mime = MIME_VIDEO_MPEG, .media_class = DLNA_CLASS_AV,.id = "MPEG1",},
  &(dlna_profile_t) { .ext = ".m2v", .mime = MIME_VIDEO_MPEG_2, .media_class = DLNA_CLASS_AV,.id = "MPEG_PS_PAL",},
  &(dlna_profile_t) { .ext = ".mpg2", .mime = MIME_VIDEO_MPEG_2, .media_class = DLNA_CLASS_AV,.id = "MPEG_PS_PAL",},
  &(dlna_profile_t) { .ext = ".mpeg2", .mime = MIME_VIDEO_MPEG_2, .media_class = DLNA_CLASS_AV,.id = "MPEG_PS_PAL",},
  &(dlna_profile_t) { .ext = ".ps", .mime = MIME_VIDEO_MPEG_2, .media_class = DLNA_CLASS_AV,.id = "MPEG_PS_PAL",},
  &(dlna_profile_t) { .ext = ".m4v", .mime = MIME_VIDEO_MPEG_4, .media_class = DLNA_CLASS_AV,.id = "AVC_MP4_MP_SD_AC3",},
  &(dlna_profile_t) { .ext = ".m4p", .mime = MIME_VIDEO_MPEG_4, .media_class = DLNA_CLASS_AV,.id = "AVC_MP4_MP_SD_AC3",},
  &(dlna_profile_t) { .ext = ".mp4", .mime = MIME_VIDEO_MPEG_4, .media_class = DLNA_CLASS_AV,.id = "MPEG4_P2_MP4_SP_AAC",},
  &(dlna_profile_t) { .ext = ".mp4ps", .mime = "video/x-nerodigital-ps", .media_class = DLNA_CLASS_AV,.id = "MPEG4_P2_MP4_SP_AAC",},
  &(dlna_profile_t) { .ext = ".ts", .mime = MIME_VIDEO_MPEG_2, .media_class = DLNA_CLASS_AV,.id = "MPEG_TS_SD_EU",},
  &(dlna_profile_t) { .ext = ".ogm", .mime = MIME_VIDEO_MPEG, .media_class = DLNA_CLASS_AV,},
  &(dlna_profile_t) { .ext = ".mkv", .mime = MIME_VIDEO_MPEG, .media_class = DLNA_CLASS_AV,},
  &(dlna_profile_t) { .ext = ".rmvb", .mime = MIME_VIDEO_MPEG, .media_class = DLNA_CLASS_AV,},
  &(dlna_profile_t) { .ext = ".mov", .mime = MIME_VIDEO_QT, .media_class = DLNA_CLASS_AV,.id = "AVC_MP4_MP_SD_AC3",},
  &(dlna_profile_t) { .ext = ".hdmov", .mime = MIME_VIDEO_QT, .media_class = DLNA_CLASS_AV,.id = "AVC_MP4_MP_SD_AC3",},
  &(dlna_profile_t) { .ext = ".qt", .mime = MIME_VIDEO_QT, .media_class = DLNA_CLASS_AV,.id = "AVC_MP4_MP_SD_AC3",},
  &(dlna_profile_t) { .ext = ".bin", .mime = MIME_VIDEO_MPEG_2, .media_class = DLNA_CLASS_AV,.id = "MPEG_PS_PAL",},
  &(dlna_profile_t) { .ext = ".iso", .mime = MIME_VIDEO_MPEG_2, .media_class = DLNA_CLASS_AV,.id = "MPEG_PS_PAL",},

  /* Audio files */
  &(dlna_profile_t) { .ext = ".3gp", .mime = MIME_AUDIO_3GP, .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".aac", .mime = MIME_AUDIO_AAC, .media_class = DLNA_CLASS_AUDIO,.id = "AAC_ISO",},
  &(dlna_profile_t) { .ext = ".ac3", .mime = MIME_AUDIO_AC3, .media_class = DLNA_CLASS_AUDIO,.id = "AC3",},
//  { "ac3", .mime = MIME_AUDIO_DOLBY_DIGITAL, .media_class = DLNA_CLASS_AUDIO,.id = "AC3",},
  &(dlna_profile_t) { .ext = ".aif", .mime = MIME_AUDIO_AIF, .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".aiff", .mime = MIME_AUDIO_AIF, .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".at3p", .mime = "audio/x-atrac3", .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".au", .mime = "audio/basic", .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".snd", .mime = "audio/basic", .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".dts", .mime = "audio/x-dts", .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".rmi", .mime = MIME_AUDIO_MIDI, .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".mid", .mime = MIME_AUDIO_MIDI, .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".mp1", .mime = "audio/mp1", .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".mp2", .mime = "audio/mp2", .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".mp3", .mime = MIME_AUDIO_MPEG, .media_class = DLNA_CLASS_AUDIO,.id = "MP3",},
  &(dlna_profile_t) { .ext = ".m4a", .mime = MIME_AUDIO_MPEG_4, .media_class = DLNA_CLASS_AUDIO,.id = "AAC_ADTS",},
  &(dlna_profile_t) { .ext = ".ogg", .mime = MIME_AUDIO_OGG, .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".wav", .mime = MIME_AUDIO_WAV, .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".pcm", .mime = MIME_AUDIO_LPCM, .media_class = DLNA_CLASS_AUDIO,.id = "LPCM",},
  &(dlna_profile_t) { .ext = ".lpcm", .mime = MIME_AUDIO_LPCM, .media_class = DLNA_CLASS_AUDIO,.id = "LPCM",},
  &(dlna_profile_t) { .ext = ".l16", .mime = MIME_AUDIO_LPCM, .media_class = DLNA_CLASS_AUDIO,.id = "LPCM_low",},
  &(dlna_profile_t) { .ext = ".wma", .mime = MIME_AUDIO_WMA, .media_class = DLNA_CLASS_AUDIO,.id = "WMABASE",},
  &(dlna_profile_t) { .ext = ".mka", .mime = MIME_AUDIO_MPEG, .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".ra", .mime = MIME_AUDIO_REAL, .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".rm", .mime = MIME_AUDIO_REAL, .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".ram", .mime = MIME_AUDIO_REAL, .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".flac", .mime = MIME_AUDIO_FLAC, .media_class = DLNA_CLASS_AUDIO,},
  &(dlna_profile_t) { .ext = ".acm", .mime = MIME_AUDIO_ATRAC, .media_class = DLNA_CLASS_AUDIO,.id = "ATRAC3plus",},

  /* Images files */
  &(dlna_profile_t) { .ext = ".bmp", .mime = MIME_IMAGE_BMP, .media_class = DLNA_CLASS_IMAGE,},
  &(dlna_profile_t) { .ext = ".ico", .mime = "image/x-icon", .media_class = DLNA_CLASS_IMAGE,},
  &(dlna_profile_t) { .ext = ".gif", .mime = MIME_IMAGE_GIF, .media_class = DLNA_CLASS_IMAGE,},
  &(dlna_profile_t) { .ext = ".jpeg", .mime = MIME_IMAGE_JPEG, .media_class = DLNA_CLASS_IMAGE,.id = "JPEG_LRG",},
  &(dlna_profile_t) { .ext = ".jpg", .mime = MIME_IMAGE_JPEG, .media_class = DLNA_CLASS_IMAGE,.id = "JPEG_MED",},
  &(dlna_profile_t) { .ext = ".jpe", .mime = MIME_IMAGE_JPEG, .media_class = DLNA_CLASS_IMAGE,.id = "JPEG_SML",},
  &(dlna_profile_t) { .ext = ".pcd", .mime = "image/x-ms-bmp", .media_class = DLNA_CLASS_IMAGE,},
  &(dlna_profile_t) { .ext = ".png", .mime = MIME_IMAGE_PNG, .media_class = DLNA_CLASS_IMAGE,.id = "PNG_LRG",},
  &(dlna_profile_t) { .ext = ".pnm", .mime = "image/x-portable-anymap", .media_class = DLNA_CLASS_IMAGE,},
  &(dlna_profile_t) { .ext = ".ppm", .mime = "image/x-portable-pixmap", .media_class = DLNA_CLASS_IMAGE,},
  &(dlna_profile_t) { .ext = ".qti", .mime = MIME_IMAGE_QT, .media_class = DLNA_CLASS_IMAGE,},
  &(dlna_profile_t) { .ext = ".qtf", .mime = MIME_IMAGE_QT, .media_class = DLNA_CLASS_IMAGE,},
  &(dlna_profile_t) { .ext = ".qtif", .mime = MIME_IMAGE_QT, .media_class = DLNA_CLASS_IMAGE,},
  &(dlna_profile_t) { .ext = ".tif", .mime = MIME_IMAGE_TIFF, .media_class = DLNA_CLASS_IMAGE,},
  &(dlna_profile_t) { .ext = ".tiff", .mime = MIME_IMAGE_TIFF, .media_class = DLNA_CLASS_IMAGE,},

  NULL
};

static const dlna_profile_t **
upnp_get_supported_media_profiles ()
{
  return profiles_list;
}

static char *
get_file_extension (const char *filename)
{
  char *str = NULL;

  str = strrchr (filename, '.');
  return str;
}

static const dlna_profile_t *
upnp_get_media_profile (char *profileid)
{
  int i;

  for (i = 0; profiles_list[i]; i++)
  {
    const dlna_profile_t *profile = profiles_list[i];
    if (!strcmp(profileid, profile->id))
      return profile;
  }
  return NULL;
}

static const dlna_profile_t *
upnp_guess_media_profile (dlna_stream_t *reader, void **cookie dlna_unused)
{
  char *extension = NULL;
  int i;

  extension = get_file_extension (reader->url);
  if (!extension)
    return NULL;
  for (i = 0; profiles_list[i]; i++)
  {
    const dlna_profile_t *profile = profiles_list[i];
    if (!strcmp (extension, profile->ext))
    {
      return profile;
    }
  }
  return NULL;
}

static void
upnp_free ()
{
}

dlna_profiler_t upnpav_profiler =
{
  .guess_media_profile = upnp_guess_media_profile,
  .get_media_profile = upnp_get_media_profile,
  .get_supported_media_profiles = upnp_get_supported_media_profiles,
  .free = upnp_free,
};

