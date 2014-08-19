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
#include "profiles.h"
#include "containers.h"

extern dlna_item_t *dms_db_get (dlna_t *dlna, uint32_t id);

static dlna_properties_t *dlna_item_get_properties (dlna_item_t *item);
static dlna_metadata_t *dlna_item_get_metadata (dlna_item_t *item);

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

extern dlna_registered_profile_t dlna_profile_image_jpeg;
extern dlna_registered_profile_t dlna_profile_image_png;
extern dlna_registered_profile_t dlna_profile_audio_ac3;
extern dlna_registered_profile_t dlna_profile_audio_amr;
extern dlna_registered_profile_t dlna_profile_audio_atrac3;
extern dlna_registered_profile_t dlna_profile_audio_lpcm;
extern dlna_registered_profile_t dlna_profile_audio_mp3;
extern dlna_registered_profile_t dlna_profile_audio_mpeg4;
extern dlna_registered_profile_t dlna_profile_audio_wma;
extern dlna_registered_profile_t dlna_profile_av_mpeg1;
extern dlna_registered_profile_t dlna_profile_av_mpeg2;
extern dlna_registered_profile_t dlna_profile_av_mpeg4_part2;
extern dlna_registered_profile_t dlna_profile_av_mpeg4_part10;
extern dlna_registered_profile_t dlna_profile_av_wmv9;

static void
dlna_register_profile (dlna_t *dlna, dlna_registered_profile_t *profile)
{
  void **p;

  if (!dlna)
    return;

  if (!dlna->inited)
    dlna = dlna_init ();
  
  p = &dlna->first_profile;
  while (*p != NULL)
  {
    if (((dlna_registered_profile_t *) *p)->id == profile->id)
      return; /* already registered */
    p = (void *) &((dlna_registered_profile_t *) *p)->next;
  }
  *p = profile;
  profile->next = NULL;
}

void
dlna_register_all_media_profiles (dlna_t *dlna)
{
  if (!dlna)
    return;
  
  if (!dlna->inited)
    dlna = dlna_init ();
  
  dlna_register_profile (dlna, &dlna_profile_image_jpeg);
  dlna_register_profile (dlna, &dlna_profile_image_png);
  dlna_register_profile (dlna, &dlna_profile_audio_ac3);
  dlna_register_profile (dlna, &dlna_profile_audio_amr);
  dlna_register_profile (dlna, &dlna_profile_audio_atrac3);
  dlna_register_profile (dlna, &dlna_profile_audio_lpcm);
  dlna_register_profile (dlna, &dlna_profile_audio_mp3);
  dlna_register_profile (dlna, &dlna_profile_audio_mpeg4);
  dlna_register_profile (dlna, &dlna_profile_audio_wma);
  dlna_register_profile (dlna, &dlna_profile_av_mpeg1);
  dlna_register_profile (dlna, &dlna_profile_av_mpeg2);
  dlna_register_profile (dlna, &dlna_profile_av_mpeg4_part2);
  dlna_register_profile (dlna, &dlna_profile_av_mpeg4_part10);
  dlna_register_profile (dlna, &dlna_profile_av_wmv9);
}

void
dlna_register_media_profile (dlna_t *dlna, dlna_media_profile_t profile)
{
  if (!dlna)
    return;
  
  if (!dlna->inited)
    dlna = dlna_init ();
  
  switch (profile)
  {
  case DLNA_PROFILE_IMAGE_JPEG:
    dlna_register_profile (dlna, &dlna_profile_image_jpeg);
    break;
  case DLNA_PROFILE_IMAGE_PNG:
    dlna_register_profile (dlna, &dlna_profile_image_png);
    break;
  case DLNA_PROFILE_AUDIO_AC3:
    dlna_register_profile (dlna, &dlna_profile_audio_ac3);
    break;
  case DLNA_PROFILE_AUDIO_AMR:
    dlna_register_profile (dlna, &dlna_profile_audio_amr);
    break;
  case DLNA_PROFILE_AUDIO_ATRAC3:
    dlna_register_profile (dlna, &dlna_profile_audio_atrac3);
    break;
  case DLNA_PROFILE_AUDIO_LPCM:
    dlna_register_profile (dlna, &dlna_profile_audio_lpcm);
    break;
  case DLNA_PROFILE_AUDIO_MP3:
    dlna_register_profile (dlna, &dlna_profile_audio_mp3);
    break;
  case DLNA_PROFILE_AUDIO_MPEG4:
    dlna_register_profile (dlna, &dlna_profile_audio_mpeg4);
    break;
  case DLNA_PROFILE_AUDIO_WMA:
    dlna_register_profile (dlna, &dlna_profile_audio_wma);
    break;
  case DLNA_PROFILE_AV_MPEG1:
    dlna_register_profile (dlna, &dlna_profile_av_mpeg1);
    break;
  case DLNA_PROFILE_AV_MPEG2:
    dlna_register_profile (dlna, &dlna_profile_av_mpeg2);
    break;
  case DLNA_PROFILE_AV_MPEG4_PART2:
    dlna_register_profile (dlna, &dlna_profile_av_mpeg4_part2);
    break;
  case DLNA_PROFILE_AV_MPEG4_PART10:
    dlna_register_profile (dlna, &dlna_profile_av_mpeg4_part10);
    break;
  case DLNA_PROFILE_AV_WMV9:
    dlna_register_profile (dlna, &dlna_profile_av_wmv9);
    break;
  default:
    break;
  }
}

static int
dlna_is_profile_registered (dlna_t *dlna, dlna_media_profile_t profile)
{
  void **p;

  return 1;
  
  if (!dlna)
    return 0;

  p = &dlna->first_profile;
  while (*p)
  {
    if (((dlna_registered_profile_t *) *p)->id == profile)
      return 1;
    p = (void *) &((dlna_registered_profile_t *) *p)->next;
  }

  return 0;
}

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

char **
dlna_get_supported_mime_types (dlna_t *dlna)
{
  char **mimes;
  int i = 0;
  
  if (!dlna)
    return NULL;

  mimes = malloc (sizeof (char *));
  *mimes = NULL;

  switch (dlna->mode)
  {
  case DLNA_CAPABILITY_DLNA:
    if (dlna_is_profile_registered (dlna, DLNA_PROFILE_IMAGE_JPEG))
      mimes = dlna_list_add (mimes, MIME_IMAGE_JPEG);

    if (dlna_is_profile_registered (dlna, DLNA_PROFILE_IMAGE_PNG))
      mimes = dlna_list_add (mimes, MIME_IMAGE_PNG);
    
    if (dlna_is_profile_registered (dlna, DLNA_PROFILE_AUDIO_AC3))
      mimes = dlna_list_add (mimes, MIME_AUDIO_DOLBY_DIGITAL);
    
    if (dlna_is_profile_registered (dlna, DLNA_PROFILE_AUDIO_AMR))
    {
      mimes = dlna_list_add (mimes, MIME_AUDIO_MPEG_4);
      mimes = dlna_list_add (mimes, MIME_AUDIO_3GP);
    }
    
    if (dlna_is_profile_registered (dlna, DLNA_PROFILE_AUDIO_ATRAC3))
      mimes = dlna_list_add (mimes, MIME_AUDIO_ATRAC);
    
    if (dlna_is_profile_registered (dlna, DLNA_PROFILE_AUDIO_LPCM))
      mimes = dlna_list_add (mimes, MIME_AUDIO_LPCM);
    
    if (dlna_is_profile_registered (dlna, DLNA_PROFILE_AUDIO_MP3))
      mimes = dlna_list_add (mimes, MIME_AUDIO_MPEG);
    
    if (dlna_is_profile_registered (dlna, DLNA_PROFILE_AUDIO_MPEG4))
    {
      mimes = dlna_list_add (mimes, MIME_AUDIO_ADTS);
      mimes = dlna_list_add (mimes, MIME_AUDIO_MPEG_4);
    }
    
    if (dlna_is_profile_registered (dlna, DLNA_PROFILE_AUDIO_WMA))
      mimes = dlna_list_add (mimes, MIME_AUDIO_WMA);
    
    if (dlna_is_profile_registered (dlna, DLNA_PROFILE_AV_MPEG1))
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG);
    
    if (dlna_is_profile_registered (dlna, DLNA_PROFILE_AV_MPEG2))
    {
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG);
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG_TS);
    }
    
    if (dlna_is_profile_registered (dlna, DLNA_PROFILE_AV_MPEG4_PART2))
    {
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG);
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG_4);
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG_TS);
    }
    
    if (dlna_is_profile_registered (dlna, DLNA_PROFILE_AV_MPEG4_PART10))
    {
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG);
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG_4);
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG_TS);
    }
    
    if (dlna_is_profile_registered (dlna, DLNA_PROFILE_AV_WMV9))
      mimes = dlna_list_add (mimes, MIME_VIDEO_WMV);
    break;

  case DLNA_CAPABILITY_UPNP_AV:
  case DLNA_CAPABILITY_UPNP_AV_XBOX:
    for (i = 0; mime_type_list[i].profile.mime; i++)
      mimes = dlna_list_add (mimes, (char *) mime_type_list[i].profile.mime);
    break;

  default:
    break;
  }

  mimes = dlna_list_add (mimes, NULL);
  return mimes;
}

static av_codecs_t *
av_profile_get_codecs (AVFormatContext *ctx)
{
  av_codecs_t *codecs = NULL;
  unsigned int i;
  int audio_stream = -1, video_stream = -1;
 
  codecs = malloc (sizeof (av_codecs_t));
  codecs->nb_streams = ctx->nb_streams;

  for (i = 0; i < codecs->nb_streams; i++)
  {
    if (audio_stream == -1 &&
        ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
    {
      audio_stream = i;
      continue;
    }
    else if (video_stream == -1 &&
             ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
    {
      video_stream = i;
      continue;
    }
  }

  codecs->as = audio_stream >= 0 ? ctx->streams[audio_stream] : NULL;
  codecs->ac = audio_stream >= 0 ? ctx->streams[audio_stream]->codec : NULL;

  codecs->vs = video_stream >= 0 ? ctx->streams[video_stream] : NULL;
  codecs->vc = video_stream >= 0 ? ctx->streams[video_stream]->codec : NULL;

  /* check for at least one video stream and one audio stream in container */
  if (!codecs->ac && !codecs->vc)
  {
    free (codecs);
    return NULL;
  }
  
  return codecs;
}

static int
match_file_extension (const char *filename, const char *extensions)
{
  const char *ext, *p;
  char ext1[32], *q;

  if (!filename)
    return 0;

  ext = strrchr (filename, '.');
  if (ext)
  {
    ext++;
    p = extensions;
    for (;;)
    {
      q = ext1;
      while (*p != '\0' && *p != ',' && (q - ext1 < (int) sizeof (ext1) - 1))
        *q++ = *p++;
      *q = '\0';
      if (!strcasecmp (ext1, ext))
        return 1;
      if (*p == '\0')
        break;
      p++;
    }
  }
  
  return 0;
}

static void
dlna_media_profile_free(dlna_item_t *item)
{
  AVFormatContext *ctx = (AVFormatContext *)item->profile_cookie;
  avformat_close_input (&ctx);
}

dlna_profile_t *
dlna_get_media_profile (dlna_t *dlna, char *profileid)
{
  int i = 0;
  dlna_registered_profile_t *p;

  if (!profileid)
	return NULL;
  p = dlna->first_profile;
  while (p)
  {
    dlna_profile_t *prof;
    i = 0;
    while ((prof = p->profiles[i]) != NULL)
    {
      if (!strcmp(profileid, prof->id))
      {
        if (prof->media_class == DLNA_CLASS_UNKNOWN)
          prof->media_class = p->class;
        return prof;
      }
      i++;
    }
    p = p->next;
  }
  for (i = 0; mime_type_list[i].profile.mime; i++)
    if (!strcmp(profileid, mime_type_list[i].extension))
      return &mime_type_list[i].profile;
  return NULL;
}

dlna_profile_t *
dlna_guess_media_profile (dlna_t *dlna, dlna_item_t *item)
{
  dlna_registered_profile_t *p;
  dlna_profile_t *profile = NULL;
  AVFormatContext *ctx = NULL;
  av_codecs_t *codecs;

  if (!dlna)
    return NULL;
  
  if (!dlna->inited)
    dlna = dlna_init ();
  
  if (avformat_open_input (&ctx, item->filename, NULL, NULL) != 0)
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL, "can't open file: %s\n", item->filename);
    return NULL;
  }

  if (avformat_find_stream_info (ctx, NULL) < 0)
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL, "can't find stream info\n");
    avformat_close_input (&ctx);
    return NULL;
  }

  /* grab codecs info */
  codecs = av_profile_get_codecs (ctx);
  if (!codecs)
    return NULL;

#ifdef HAVE_DEBUG
  av_dump_format (ctx, 0, NULL, 0);
#endif /* HAVE_DEBUG */

  p = dlna->first_profile;
  while (p)
  {
    dlna_profile_t *prof;
    
    if (dlna->check_extensions)
    {
      if (p->extensions)
      {
        /* check for valid file extension */
        if (!match_file_extension (item->filename, p->extensions))
        {
          p = p->next;
          continue;
        }
      }
    }
    
    prof = p->probe (ctx, codecs);
    if (prof)
    {
      profile = prof;
      profile->media_class = p->class;
      break;
    }
    p = p->next;
  }

  profile->get_properties = dlna_item_get_properties;
  profile->get_metadata = dlna_item_get_metadata;
  profile->free = dlna_media_profile_free;
  item->profile_cookie = ctx;
  free (codecs);
  return profile;
}

static dlna_profile_t *

upnp_guess_media_profile (dlna_t *dlna, dlna_item_t *item)
{
  dlna_profile_t *profile = NULL;
  char *extension;
  int i;
  
  if (!dlna)
    return NULL;

  extension = get_file_extension (item->filename);
  if (!extension)
    return NULL;
  
  for (i = 0; mime_type_list[i].extension; i++)
    if (!strcmp (extension, mime_type_list[i].extension))
      profile = &mime_type_list[i].profile;

  return profile;
}

static dlna_properties_t *
dlna_item_get_properties (dlna_item_t *item)
{
  AVFormatContext *ctx = (AVFormatContext *)item->profile_cookie;
  dlna_properties_t *prop;
  int duration, hours, min, sec;
  av_codecs_t *codecs;

  if (!ctx)
    return NULL;

  prop = malloc (sizeof (dlna_properties_t));

  duration = (int) (ctx->duration / AV_TIME_BASE);
  hours = (int) (duration / 3600);
  min = (int) ((duration - (hours * 3600)) / 60);
  sec = (int) (duration - (hours * 3600) - (min * 60));
  memset (prop->duration, '\0', 64);
  if (hours)
    sprintf (prop->duration, "%d:%.2d:%.2d.", hours, min, sec);
  else
    sprintf (prop->duration, ":%.2d:%.2d.", min, sec);

  /* grab codecs info */
  codecs = av_profile_get_codecs (ctx);
  if (!codecs)
    return NULL;
  prop->bitrate = (uint32_t) (ctx->bit_rate / 8);
  prop->sample_frequency = codecs->ac ? codecs->ac->sample_rate : 0;
  prop->bps = codecs->ac ? codecs->ac->bits_per_raw_sample : 0;
  prop->channels = codecs->ac ? codecs->ac->channels : 0;

  memset (prop->resolution, '\0', 64);
  if (codecs->vc)
    sprintf (prop->resolution, "%dx%d",
             codecs->vc->width, codecs->vc->height);

  free (codecs);
  return prop;
}

static dlna_metadata_t *
dlna_item_get_metadata (dlna_item_t *item)
{
  AVFormatContext *ctx = (AVFormatContext *)item->profile_cookie;
  dlna_metadata_t *meta;
  AVDictionary *dict = ctx->metadata;
  AVDictionaryEntry *entry;
  
  if (!ctx)
    return NULL;

  meta = malloc (sizeof (dlna_metadata_t));
  memset(meta, 0, sizeof (dlna_metadata_t));
  entry = av_dict_get(dict, "title", NULL, 0);
  if (entry && entry->value)
    meta->title   = strdup (entry->value);
  entry = av_dict_get(dict, "author", NULL, 0);
  if (entry && entry->value)
    meta->author  = strdup (entry->value);
  else
  {
    entry = av_dict_get(dict, "artist", NULL, 0);
    if (entry && entry->value)
      meta->author  = strdup (entry->value);
  }
  entry = av_dict_get(dict, "comment", NULL, 0);
  if (entry && entry->value)
    meta->comment = strdup (entry->value);
  entry = av_dict_get(dict, "album", NULL, 0);
  if (entry && entry->value)
    meta->album   = strdup (entry->value);
  entry = av_dict_get(dict, "track", NULL, 0);
  if (entry && entry->value)
    meta->track   = atoi(entry->value);
  entry = av_dict_get(dict, "genre", NULL, 0);
  if (entry && entry->value)
    meta->genre   = strdup (entry->value);

  return meta;
}

static void
dlna_metadata_free (dlna_metadata_t *meta)
{
  if (!meta)
    return;

  if (meta->title)
    free (meta->title);
  if (meta->author)
    free (meta->author);
  if (meta->comment)
    free (meta->comment);
  if (meta->album)
    free (meta->album);
  if (meta->genre)
    free (meta->genre);
  free (meta);
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

  if (stat (filename, &st))
    return NULL;
  
  item = calloc (1, sizeof (dlna_item_t));

  item->filename   = strdup (filename);
  item->filesize   = st.st_size;
  if (dlna->mode == DLNA_CAPABILITY_DLNA)
    item->profile    = dlna_guess_media_profile (dlna, item);
  else
    item->profile    = upnp_guess_media_profile (dlna, item);
  if (!item->profile) /* not DLNA compliant */
  {
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
  dlna_metadata_free (item->metadata);
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

int
stream_ctx_is_image (AVFormatContext *ctx,
                     av_codecs_t *codecs)
{
  /* should only have 1 stream */
  if (ctx->nb_streams > 1)
    return 0;

  /* should be inside image container */
  if (stream_get_container (ctx) != CT_IMAGE)
    return 0;

  if (!codecs->vc)
    return 0;

  return 1;
}

int
stream_ctx_is_audio (av_codecs_t *codecs)
{
  /* we need an audio codec ... */
  if (!codecs->ac)
    return 0;

  /* ... but no video one */
  if (codecs->vc)
    return 0;

  return 1;
}

int
stream_ctx_is_av (av_codecs_t *codecs)
{
  if (!codecs->as || !codecs->ac || !codecs->vs || !codecs->vc)
    return 0;

  return 1;
}

char *
get_file_extension (const char *filename)
{
  char *str = NULL;

  str = strrchr (filename, '.');
  if (str)
    str++;

  return str;
}

audio_profile_t
audio_profile_guess (AVCodecContext *ac)
{
  audio_profile_t ap = AUDIO_PROFILE_INVALID;
  
  if (!ac)
    return ap;

  ap = audio_profile_guess_aac (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_ac3 (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_amr (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_atrac (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_g726 (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_lpcm (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_mp2 (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_mp3 (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_wma (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  return AUDIO_PROFILE_INVALID;
}
