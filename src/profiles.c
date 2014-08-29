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
#include "ffmpeg_profiler/ffmpeg_profiler.h"

extern dlna_item_t *dms_db_get (dlna_t *dlna, uint32_t id);

typedef struct mime_type_s {
  const char *extension;
  dlna_profile_t profile;
} mime_type_t;

static const mime_type_t mime_type_list[] = {
  /* Video files */
  { "asf",   {.mime = "video/x-ms-asf", .media_class = DLNA_CLASS_AV,}},
  { "avc",   {.mime = "video/avi", .media_class = DLNA_CLASS_AV,}},
  { "avi",   {.mime = "video/avi", .media_class = DLNA_CLASS_AV,}},
  { "dv",    {.mime = "video/x-dv", .media_class = DLNA_CLASS_AV,}},
  { "divx",  {.mime = "video/avi", .media_class = DLNA_CLASS_AV,}},
  { "wmv",   {.mime = "video/x-ms-wmv", .media_class = DLNA_CLASS_AV,}},
  { "mjpg",  {.mime = "video/x-motion-jpeg", .media_class = DLNA_CLASS_AV,}},
  { "mjpeg", {.mime = "video/x-motion-jpeg", .media_class = DLNA_CLASS_AV,}},
  { "mpeg",  {.mime = "video/mpeg", .media_class = DLNA_CLASS_AV,}},
  { "mpg",   {.mime = "video/mpeg", .media_class = DLNA_CLASS_AV,}},
  { "mpe",   {.mime = "video/mpeg", .media_class = DLNA_CLASS_AV,}},
  { "mp2p",  {.mime = "video/mp2p", .media_class = DLNA_CLASS_AV,}},
  { "vob",   {.mime = "video/mp2p", .media_class = DLNA_CLASS_AV,}},
  { "mp2t",  {.mime = "video/mp2t", .media_class = DLNA_CLASS_AV,}},
  { "m1v",   {.mime = "video/mpeg", .media_class = DLNA_CLASS_AV,}},
  { "m2v",   {.mime = "video/mpeg2", .media_class = DLNA_CLASS_AV,}},
  { "mpg2",  {.mime = "video/mpeg2", .media_class = DLNA_CLASS_AV,}},
  { "mpeg2", {.mime = "video/mpeg2", .media_class = DLNA_CLASS_AV,}},
  { "m4v",   {.mime = "video/mp4", .media_class = DLNA_CLASS_AV,}},
  { "m4p",   {.mime = "video/mp4", .media_class = DLNA_CLASS_AV,}},
  { "mp4",   {.mime = "video/mp4", .media_class = DLNA_CLASS_AV,}},
  { "mp4ps", {.mime = "video/x-nerodigital-ps", .media_class = DLNA_CLASS_AV,}},
  { "ts",    {.mime = "video/mpeg2", .media_class = DLNA_CLASS_AV,}},
  { "ogm",   {.mime = "video/mpeg", .media_class = DLNA_CLASS_AV,}},
  { "mkv",   {.mime = "video/mpeg", .media_class = DLNA_CLASS_AV,}},
  { "rmvb",  {.mime = "video/mpeg", .media_class = DLNA_CLASS_AV,}},
  { "mov",   {.mime = "video/quicktime", .media_class = DLNA_CLASS_AV,}},
  { "hdmov", {.mime = "video/quicktime", .media_class = DLNA_CLASS_AV,}},
  { "qt",    {.mime = "video/quicktime", .media_class = DLNA_CLASS_AV,}},
  { "bin",   {.mime = "video/mpeg2", .media_class = DLNA_CLASS_AV,}},
  { "iso",   {.mime = "video/mpeg2", .media_class = DLNA_CLASS_AV,}},

  /* Audio files */
  { "3gp",  {.mime = "audio/3gpp", .media_class = DLNA_CLASS_AUDIO,}},
  { "aac",  {.mime = "audio/x-aac", .media_class = DLNA_CLASS_AUDIO,}},
  { "ac3",  {.mime = "audio/x-ac3", .media_class = DLNA_CLASS_AUDIO,}},
  { "aif",  {.mime = "audio/aiff", .media_class = DLNA_CLASS_AUDIO,}},
  { "aiff", {.mime = "audio/aiff", .media_class = DLNA_CLASS_AUDIO,}},
  { "at3p", {.mime = "audio/x-atrac3", .media_class = DLNA_CLASS_AUDIO,}},
  { "au",   {.mime = "audio/basic", .media_class = DLNA_CLASS_AUDIO,}},
  { "snd",  {.mime = "audio/basic", .media_class = DLNA_CLASS_AUDIO,}},
  { "dts",  {.mime = "audio/x-dts", .media_class = DLNA_CLASS_AUDIO,}},
  { "rmi",  {.mime = "audio/midi", .media_class = DLNA_CLASS_AUDIO,}},
  { "mid",  {.mime = "audio/midi", .media_class = DLNA_CLASS_AUDIO,}},
  { "mp1",  {.mime = "audio/mp1", .media_class = DLNA_CLASS_AUDIO,}},
  { "mp2",  {.mime = "audio/mp2", .media_class = DLNA_CLASS_AUDIO,}},
  { "mp3",  {.mime = "audio/mpeg", .media_class = DLNA_CLASS_AUDIO,}},
  { "m4a",  {.mime = "audio/mp4", .media_class = DLNA_CLASS_AUDIO,}},
  { "ogg",  {.mime = "audio/x-ogg", .media_class = DLNA_CLASS_AUDIO,}},
  { "wav",  {.mime = "audio/wav", .media_class = DLNA_CLASS_AUDIO,}},
  { "pcm",  {.mime = "audio/l16", .media_class = DLNA_CLASS_AUDIO,}},
  { "lpcm", {.mime = "audio/l16", .media_class = DLNA_CLASS_AUDIO,}},
  { "l16",  {.mime = "audio/l16", .media_class = DLNA_CLASS_AUDIO,}},
  { "wma",  {.mime = "audio/x-ms-wma", .media_class = DLNA_CLASS_AUDIO,}},
  { "mka",  {.mime = "audio/mpeg", .media_class = DLNA_CLASS_AUDIO,}},
  { "ra",   {.mime = "audio/x-pn-realaudio", .media_class = DLNA_CLASS_AUDIO,}},
  { "rm",   {.mime = "audio/x-pn-realaudio", .media_class = DLNA_CLASS_AUDIO,}},
  { "ram",  {.mime = "audio/x-pn-realaudio", .media_class = DLNA_CLASS_AUDIO,}},
  { "flac", {.mime = "audio/x-flac", .media_class = DLNA_CLASS_AUDIO,}},

  /* Images files */
  { "bmp",  {.mime = "image/bmp", .media_class = DLNA_CLASS_IMAGE,}},
  { "ico",  {.mime = "image/x-icon", .media_class = DLNA_CLASS_IMAGE,}},
  { "gif",  {.mime = "image/gif", .media_class = DLNA_CLASS_IMAGE,}},
  { "jpeg", {.mime = "image/jpeg", .media_class = DLNA_CLASS_IMAGE,}},
  { "jpg",  {.mime = "image/jpeg", .media_class = DLNA_CLASS_IMAGE,}},
  { "jpe",  {.mime = "image/jpeg", .media_class = DLNA_CLASS_IMAGE,}},
  { "pcd",  {.mime = "image/x-ms-bmp", .media_class = DLNA_CLASS_IMAGE,}},
  { "png",  {.mime = "image/png", .media_class = DLNA_CLASS_IMAGE,}},
  { "pnm",  {.mime = "image/x-portable-anymap", .media_class = DLNA_CLASS_IMAGE,}},
  { "ppm",  {.mime = "image/x-portable-pixmap", .media_class = DLNA_CLASS_IMAGE,}},
  { "qti",  {.mime = "image/x-quicktime", .media_class = DLNA_CLASS_IMAGE,}},
  { "qtf",  {.mime = "image/x-quicktime", .media_class = DLNA_CLASS_IMAGE,}},
  { "qtif", {.mime = "image/x-quicktime", .media_class = DLNA_CLASS_IMAGE,}},
  { "tif",  {.mime = "image/tiff", .media_class = DLNA_CLASS_IMAGE,}},
  { "tiff", {.mime = "image/tiff", .media_class = DLNA_CLASS_IMAGE,}},

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

static char *
get_file_extension (const char *filename)
{
  char *str = NULL;

  str = strrchr (filename, '.');
  if (str)
    str++;

  return str;
}

static char **
upnp_get_supported_mime_types ( char **mimes)
{
  int i;

  for (i = 0; mime_type_list[i].profile.mime; i++)
    mimes = dlna_list_add (mimes, (char *) mime_type_list[i].profile.mime);
  return mimes;
}

static dlna_profile_t *
upnp_get_media_profile (char *profileid)
{
  int i;

  for (i = 0; mime_type_list[i].profile.mime; i++)
    if (!strcmp(profileid, mime_type_list[i].extension))
      return &mime_type_list[i].profile;
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
      profile = &mime_type_list[i].profile;

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
dlna_item_new (dlna_t *dlna, const char *filename)
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

  item->profile    = dlna->profiler->guess_media_profile ((char *)item->filename, &item->profile_cookie);
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
