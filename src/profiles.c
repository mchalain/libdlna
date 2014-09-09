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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "dlna_internals.h"
#include "vfs.h"

extern dlna_item_t *dms_db_get (dlna_t *dlna, uint32_t id);

typedef struct mime_type_s {
  const char *extension;
  dlna_profile_t profile;
} mime_type_t;

static const mime_type_t mime_type_list[] = {
  /* Video files */
  { "asf",   {.mime = MIME_VIDEO_ASF, .media_class = DLNA_CLASS_AV,.id = "WMVMED_BASE",}},
  { "avc",   {.mime = MIME_VIDEO_AVI, .media_class = DLNA_CLASS_AV,.id = "AVC_MP4_MP_SD_AC3",}},
  { "avi",   {.mime = MIME_VIDEO_AVI, .media_class = DLNA_CLASS_AV,}},
  { "dv",    {.mime = "video/x-dv", .media_class = DLNA_CLASS_AV,}},
  { "divx",  {.mime = MIME_VIDEO_AVI, .media_class = DLNA_CLASS_AV,.id = "MPEG4_P2_MP4_SP_AAC",}},
  { "wmv",   {.mime = MIME_VIDEO_WMV, .media_class = DLNA_CLASS_AV,.id = "WMVMED_BASE",}},
  { "mjpg",  {.mime = "video/x-motion-jpeg", .media_class = DLNA_CLASS_AV,}},
  { "mjpeg", {.mime = "video/x-motion-jpeg", .media_class = DLNA_CLASS_AV,}},
  { "mpeg",  {.mime = MIME_VIDEO_MPEG, .media_class = DLNA_CLASS_AV,.id = "MPEG1",}},
  { "mpg",   {.mime = MIME_VIDEO_MPEG, .media_class = DLNA_CLASS_AV,.id = "MPEG1",}},
  { "mpe",   {.mime = MIME_VIDEO_MPEG, .media_class = DLNA_CLASS_AV,.id = "MPEG1",}},
  { "mp2p",  {.mime = "video/mp2p", .media_class = DLNA_CLASS_AV,.id = "MPEG_PS_PAL",}},
  { "vob",   {.mime = "video/mp2p", .media_class = DLNA_CLASS_AV,.id = "MPEG_PS_PAL",}},
  { "mp2t",  {.mime = "video/mp2t", .media_class = DLNA_CLASS_AV,.id = "MPEG_TS_SD_EU",}},
  { "m1v",   {.mime = MIME_VIDEO_MPEG, .media_class = DLNA_CLASS_AV,.id = "MPEG1",}},
  { "m2v",   {.mime = MIME_VIDEO_MPEG_2, .media_class = DLNA_CLASS_AV,.id = "MPEG_PS_PAL",}},
  { "mpg2",  {.mime = MIME_VIDEO_MPEG_2, .media_class = DLNA_CLASS_AV,.id = "MPEG_PS_PAL",}},
  { "mpeg2", {.mime = MIME_VIDEO_MPEG_2, .media_class = DLNA_CLASS_AV,.id = "MPEG_PS_PAL",}},
  { "m4v",   {.mime = MIME_VIDEO_MPEG_4, .media_class = DLNA_CLASS_AV,.id = "AVC_MP4_MP_SD_AC3",}},
  { "m4p",   {.mime = MIME_VIDEO_MPEG_4, .media_class = DLNA_CLASS_AV,.id = "AVC_MP4_MP_SD_AC3",}},
  { "mp4",   {.mime = MIME_VIDEO_MPEG_4, .media_class = DLNA_CLASS_AV,.id = "MPEG4_P2_MP4_SP_AAC",}},
  { "mp4ps", {.mime = "video/x-nerodigital-ps", .media_class = DLNA_CLASS_AV,.id = "MPEG4_P2_MP4_SP_AAC"}},
  { "ts",    {.mime = MIME_VIDEO_MPEG_2, .media_class = DLNA_CLASS_AV,.id = "MPEG_TS_SD_EU",}},
  { "ogm",   {.mime = MIME_VIDEO_MPEG, .media_class = DLNA_CLASS_AV,}},
  { "mkv",   {.mime = MIME_VIDEO_MPEG, .media_class = DLNA_CLASS_AV,}},
  { "rmvb",  {.mime = MIME_VIDEO_MPEG, .media_class = DLNA_CLASS_AV,}},
  { "mov",   {.mime = MIME_VIDEO_QT, .media_class = DLNA_CLASS_AV,.id = "AVC_MP4_MP_SD_AC3",}},
  { "hdmov", {.mime = MIME_VIDEO_QT, .media_class = DLNA_CLASS_AV,.id = "AVC_MP4_MP_SD_AC3",}},
  { "qt",    {.mime = MIME_VIDEO_QT, .media_class = DLNA_CLASS_AV,.id = "AVC_MP4_MP_SD_AC3",}},
  { "bin",   {.mime = MIME_VIDEO_MPEG_2, .media_class = DLNA_CLASS_AV,.id = "MPEG_PS_PAL",}},
  { "iso",   {.mime = MIME_VIDEO_MPEG_2, .media_class = DLNA_CLASS_AV,.id = "MPEG_PS_PAL",}},

  /* Audio files */
  { "3gp",  {.mime = MIME_AUDIO_3GP, .media_class = DLNA_CLASS_AUDIO,}},
  { "aac",  {.mime = MIME_AUDIO_AAC, .media_class = DLNA_CLASS_AUDIO,.id = "AAC_ISO",}},
  { "ac3",  {.mime = MIME_AUDIO_AC3, .media_class = DLNA_CLASS_AUDIO,.id = "AC3",}},
//  { "ac3",  {.mime = MIME_AUDIO_DOLBY_DIGITAL, .media_class = DLNA_CLASS_AUDIO,.id = "AC3",}},
  { "aif",  {.mime = MIME_AUDIO_AIF, .media_class = DLNA_CLASS_AUDIO,}},
  { "aiff", {.mime = MIME_AUDIO_AIF, .media_class = DLNA_CLASS_AUDIO,}},
  { "at3p", {.mime = "audio/x-atrac3", .media_class = DLNA_CLASS_AUDIO,}},
  { "au",   {.mime = "audio/basic", .media_class = DLNA_CLASS_AUDIO,}},
  { "snd",  {.mime = "audio/basic", .media_class = DLNA_CLASS_AUDIO,}},
  { "dts",  {.mime = "audio/x-dts", .media_class = DLNA_CLASS_AUDIO,}},
  { "rmi",  {.mime = MIME_AUDIO_MIDI, .media_class = DLNA_CLASS_AUDIO,}},
  { "mid",  {.mime = MIME_AUDIO_MIDI, .media_class = DLNA_CLASS_AUDIO,}},
  { "mp1",  {.mime = "audio/mp1", .media_class = DLNA_CLASS_AUDIO,}},
  { "mp2",  {.mime = "audio/mp2", .media_class = DLNA_CLASS_AUDIO,}},
  { "mp3",  {.mime = MIME_AUDIO_MPEG, .media_class = DLNA_CLASS_AUDIO,.id = "MP3",}},
  { "m4a",  {.mime = MIME_AUDIO_MPEG_4, .media_class = DLNA_CLASS_AUDIO,.id = "AAC_ADTS",}},
  { "ogg",  {.mime = MIME_AUDIO_OGG, .media_class = DLNA_CLASS_AUDIO,}},
  { "wav",  {.mime = MIME_AUDIO_WAV, .media_class = DLNA_CLASS_AUDIO,}},
  { "pcm",  {.mime = MIME_AUDIO_LPCM, .media_class = DLNA_CLASS_AUDIO,.id = "LPCM",}},
  { "lpcm", {.mime = MIME_AUDIO_LPCM, .media_class = DLNA_CLASS_AUDIO,.id = "LPCM",}},
  { "l16",  {.mime = MIME_AUDIO_LPCM, .media_class = DLNA_CLASS_AUDIO,.id = "LPCM_low",}},
  { "wma",  {.mime = MIME_AUDIO_WMA, .media_class = DLNA_CLASS_AUDIO,.id = "WMABASE",}},
  { "mka",  {.mime = MIME_AUDIO_MPEG, .media_class = DLNA_CLASS_AUDIO,}},
  { "ra",   {.mime = MIME_AUDIO_REAL, .media_class = DLNA_CLASS_AUDIO,}},
  { "rm",   {.mime = MIME_AUDIO_REAL, .media_class = DLNA_CLASS_AUDIO,}},
  { "ram",  {.mime = MIME_AUDIO_REAL, .media_class = DLNA_CLASS_AUDIO,}},
  { "flac", {.mime = MIME_AUDIO_FLAC, .media_class = DLNA_CLASS_AUDIO,}},
  { "acm",  {.mime = MIME_AUDIO_ATRAC, .media_class = DLNA_CLASS_AUDIO,.id = "ATRAC3plus"}},

  /* Images files */
  { "bmp",  {.mime = MIME_IMAGE_BMP, .media_class = DLNA_CLASS_IMAGE,}},
  { "ico",  {.mime = "image/x-icon", .media_class = DLNA_CLASS_IMAGE,}},
  { "gif",  {.mime = MIME_IMAGE_GIF, .media_class = DLNA_CLASS_IMAGE,}},
  { "jpeg", {.mime = MIME_IMAGE_JPEG, .media_class = DLNA_CLASS_IMAGE,.id = "JPEG_LRG",}},
  { "jpg",  {.mime = MIME_IMAGE_JPEG, .media_class = DLNA_CLASS_IMAGE,.id = "JPEG_MED",}},
  { "jpe",  {.mime = MIME_IMAGE_JPEG, .media_class = DLNA_CLASS_IMAGE,.id = "JPEG_SML",}},
  { "pcd",  {.mime = "image/x-ms-bmp", .media_class = DLNA_CLASS_IMAGE,}},
  { "png",  {.mime = MIME_IMAGE_PNG, .media_class = DLNA_CLASS_IMAGE,.id = "PNG_LRG"}},
  { "pnm",  {.mime = "image/x-portable-anymap", .media_class = DLNA_CLASS_IMAGE,}},
  { "ppm",  {.mime = "image/x-portable-pixmap", .media_class = DLNA_CLASS_IMAGE,}},
  { "qti",  {.mime = MIME_IMAGE_QT, .media_class = DLNA_CLASS_IMAGE,}},
  { "qtf",  {.mime = MIME_IMAGE_QT, .media_class = DLNA_CLASS_IMAGE,}},
  { "qtif", {.mime = MIME_IMAGE_QT, .media_class = DLNA_CLASS_IMAGE,}},
  { "tif",  {.mime = MIME_IMAGE_TIFF, .media_class = DLNA_CLASS_IMAGE,}},
  { "tiff", {.mime = MIME_IMAGE_TIFF, .media_class = DLNA_CLASS_IMAGE,}},

  { NULL,   {.mime = NULL}}
};

static int
dlna_list_length (void *list)
{
  void **l = list;
  int n = 0;
  while (*l++)
    n++;

  return n;
}

static void *
dlna_list_add (char **list, char *element)
{
  char **l = list;
  int n = dlna_list_length (list) + 1;
  int i;

  for (i = 0; i < n; i++)
    if (l[i] && element && !strcmp (l[i], element))
      return l;
  
  l = realloc (l, (n + 1) * sizeof (char *));
  l[n] = NULL;
  l[n - 1] = element;
  
  return l;
}

void
dlna_append_supported_mime_types (dlna_t *dlna, int sink, char *mime)
{
  if (sink)
  {
    if (!dlna->cms.sinkmimes)
    {
      dlna->cms.sinkmimes = malloc (sizeof (char*));
      *dlna->cms.sinkmimes = NULL;
	}
    dlna->cms.sinkmimes = dlna_list_add (dlna->cms.sinkmimes, mime);
  }
  else
  {
    if (!dlna->cms.sourcemimes)
    {
      dlna->cms.sourcemimes = malloc (sizeof (char*));
      *dlna->cms.sourcemimes = NULL;
	}
    dlna->cms.sourcemimes = dlna_list_add (dlna->cms.sourcemimes, mime);
  }
}

static char **
upnp_get_supported_mime_types ( char **mimes)
{
  int i;

  for (i = 0; mime_type_list[i].profile.mime; i++)
    mimes = dlna_list_add (mimes, (char *) mime_type_list[i].profile.mime);
  return mimes;
}

static char *
get_file_extension (const char *filename)
{
  char *str = NULL;

  str = strrchr (filename, '.');
  if (str)
    str++;

  return str;
}

static dlna_profile_t *
upnp_get_media_profile (char *profileid)
{
  int i;

  for (i = 0; mime_type_list[i].profile.mime; i++)
    if (!strcmp(profileid, mime_type_list[i].extension))
      return (dlna_profile_t *)&mime_type_list[i].profile;
  return NULL;
}

static dlna_profile_t *
upnp_guess_media_profile (char *filename, void **cookie)
{
  dlna_profile_t *profile = NULL;
  char *extension;
  int i;
  
  extension = get_file_extension (filename);
  if (!extension)
    return NULL;
  
  for (i = 0; mime_type_list[i].extension; i++)
    if (!strcmp (extension, mime_type_list[i].extension))
      profile = (dlna_profile_t *)&mime_type_list[i].profile;

  return profile;
}

dlna_profiler_t upnpav_profiler =
{
  .guess_media_profile = upnp_guess_media_profile,
  .get_media_profile = upnp_get_media_profile,
  .get_supported_mime_types = upnp_get_supported_mime_types,
};

/* UPnP ContentDirectory Object Item */
#define UPNP_OBJECT_ITEM_PHOTO            "object.item.imageItem.photo"
#define UPNP_OBJECT_ITEM_AUDIO            "object.item.audioItem.musicTrack"
#define UPNP_OBJECT_ITEM_VIDEO            "object.item.videoItem.movie"

char *
dlna_profile_upnp_object_item (dlna_profile_t *profile)
{
  if (!profile)
    return NULL;

  switch (profile->media_class)
  {
  case DLNA_CLASS_IMAGE:
    return UPNP_OBJECT_ITEM_PHOTO;
  case DLNA_CLASS_AUDIO:
    return UPNP_OBJECT_ITEM_AUDIO;
  case DLNA_CLASS_AV:
    return UPNP_OBJECT_ITEM_VIDEO;
  default:
    break;
  }

  return NULL;
}

char **
dlna_get_supported_mime_types (dlna_t *dlna)
{
  char **mimes;
 
  if (!dlna)
    return NULL;

  mimes = malloc (sizeof (char *));
  *mimes = NULL;

  mimes    = dlna->profiler->get_supported_mime_types (mimes);
  mimes = dlna_list_add (mimes, NULL);
  return mimes;
}

dlna_profile_t *
dlna_get_media_profile (dlna_t *dlna, char *profileid)
{
  dlna_profile_t *profile;

  if (!profileid)
	return NULL;
  profile    = dlna->profiler->get_media_profile (profileid);
  return profile;
}

dlna_item_t *
dlna_item_new (dlna_t *dlna, dlna_profiler_t *profiler, const char *filename)
{
  dlna_item_t *item;
  struct stat st;

  if (!dlna || !filename)
    return NULL;
  
  if (!dlna->inited)
    dlna = dlna_init ();

  item = calloc (1, sizeof (dlna_item_t));

  item->filename   = strdup (filename);
  if (!stat (filename, &st))
    item->filesize   = st.st_size;
  else
    item->filesize	 = -1;

  item->profile    = profiler->guess_media_profile ((char *)item->filename, &item->profile_cookie);
  if (!item->profile) /* not DLNA compliant */
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL, "can't open file: %s\n", filename);
    free (item->filename);
    free (item);
    return NULL;
  }
  if (item->profile->get_properties)
    item->properties = item->profile->get_properties (item);
  if (item->profile->get_metadata)
    item->metadata   = item->profile->get_metadata (item);
  return item;
}

void
dlna_item_free (dlna_item_t *item)
{
  if (!item)
    return;

  if (item->filename)
    free (item->filename);
  if (item->properties)
    free (item->properties);
  item->profile->free (item);
  item->profile = NULL;
  free (item);
}

dlna_item_t *
dlna_item_get(dlna_t *dlna, vfs_item_t *item)
{
	if (!item->u.resource.item)
	  item->u.resource.item = dms_db_get(dlna, item->id);
	return item->u.resource.item;
}
