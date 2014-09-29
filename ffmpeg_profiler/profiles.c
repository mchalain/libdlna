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
#include "ffmpeg_profiler.h"
#include "profiles.h"
#include "containers.h"

static dlna_properties_t *item_get_properties (dlna_item_t *item);
static dlna_metadata_t *item_get_metadata (dlna_item_t *item);
static int ffmpeg_prepare_stream (dlna_item_t *item);
static int ffmpeg_read_stream (dlna_item_t *item);

extern registered_profile_t dlna_profile_image_jpeg;
extern registered_profile_t dlna_profile_image_png;
extern registered_profile_t dlna_profile_audio_ac3;
extern registered_profile_t dlna_profile_audio_amr;
extern registered_profile_t dlna_profile_audio_atrac3;
extern registered_profile_t dlna_profile_audio_lpcm;
extern registered_profile_t dlna_profile_audio_mp3;
extern registered_profile_t dlna_profile_audio_mpeg4;
extern registered_profile_t dlna_profile_audio_wma;
extern registered_profile_t dlna_profile_av_mpeg1;
extern registered_profile_t dlna_profile_av_mpeg2;
extern registered_profile_t dlna_profile_av_mpeg4_part2;
extern registered_profile_t dlna_profile_av_mpeg4_part10;
extern registered_profile_t dlna_profile_av_wmv9;

static ffmpeg_profiler_data_t *g_profiler = NULL;

static ffmpeg_profiler_data_t *
ffmpeg_profiler_init (dlna_t *dlna dlna_unused)
{
  /* register all FFMPEG demuxers */
  av_register_all ();
  //avdevice_register_all();
  avcodec_register_all();
  avformat_network_init();

  g_profiler = calloc( 1, sizeof(ffmpeg_profiler_data_t));
  g_profiler->first_profile = NULL;
  g_profiler->inited = 1;
  ffmpeg_profiler_register_all_media_profiles ();

  return 0;
}

static void
register_profile (ffmpeg_profiler_data_t *data, registered_profile_t *profile)
{
  void **p;

  p = &data->first_profile;
  while (*p != NULL)
  {
    if (((registered_profile_t *) *p)->id == profile->id)
      return; /* already registered */
    p = (void *) &((registered_profile_t *) *p)->next;
  }
  *p = profile;
  profile->next = NULL;
}

void
ffmpeg_profiler_register_all_media_profiles ()
{
  if (!g_profiler || !g_profiler->inited)
    return ;

  register_profile (g_profiler, &dlna_profile_image_jpeg);
  register_profile (g_profiler, &dlna_profile_image_png);
  register_profile (g_profiler, &dlna_profile_audio_ac3);
  register_profile (g_profiler, &dlna_profile_audio_amr);
  register_profile (g_profiler, &dlna_profile_audio_atrac3);
  register_profile (g_profiler, &dlna_profile_audio_lpcm);
  register_profile (g_profiler, &dlna_profile_audio_mp3);
  register_profile (g_profiler, &dlna_profile_audio_mpeg4);
  register_profile (g_profiler, &dlna_profile_audio_wma);
  register_profile (g_profiler, &dlna_profile_av_mpeg1);
  register_profile (g_profiler, &dlna_profile_av_mpeg2);
  register_profile (g_profiler, &dlna_profile_av_mpeg4_part2);
  register_profile (g_profiler, &dlna_profile_av_mpeg4_part10);
  register_profile (g_profiler, &dlna_profile_av_wmv9);
}

void
ffmpeg_profiler_register_media_profile (ffmpeg_profiler_media_profile_t profile)
{
  if (!g_profiler || !g_profiler->inited)
    return;

  switch (profile)
  {
  case DLNA_PROFILE_IMAGE_JPEG:
    register_profile (g_profiler, &dlna_profile_image_jpeg);
    break;
  case DLNA_PROFILE_IMAGE_PNG:
    register_profile (g_profiler, &dlna_profile_image_png);
    break;
  case DLNA_PROFILE_AUDIO_AC3:
    register_profile (g_profiler, &dlna_profile_audio_ac3);
    break;
  case DLNA_PROFILE_AUDIO_AMR:
    register_profile (g_profiler, &dlna_profile_audio_amr);
    break;
  case DLNA_PROFILE_AUDIO_ATRAC3:
    register_profile (g_profiler, &dlna_profile_audio_atrac3);
    break;
  case DLNA_PROFILE_AUDIO_LPCM:
    register_profile (g_profiler, &dlna_profile_audio_lpcm);
    break;
  case DLNA_PROFILE_AUDIO_MP3:
    register_profile (g_profiler, &dlna_profile_audio_mp3);
    break;
  case DLNA_PROFILE_AUDIO_MPEG4:
    register_profile (g_profiler, &dlna_profile_audio_mpeg4);
    break;
  case DLNA_PROFILE_AUDIO_WMA:
    register_profile (g_profiler, &dlna_profile_audio_wma);
    break;
  case DLNA_PROFILE_AV_MPEG1:
    register_profile (g_profiler, &dlna_profile_av_mpeg1);
    break;
  case DLNA_PROFILE_AV_MPEG2:
    register_profile (g_profiler, &dlna_profile_av_mpeg2);
    break;
  case DLNA_PROFILE_AV_MPEG4_PART2:
    register_profile (g_profiler, &dlna_profile_av_mpeg4_part2);
    break;
  case DLNA_PROFILE_AV_MPEG4_PART10:
    register_profile (g_profiler, &dlna_profile_av_mpeg4_part10);
    break;
  case DLNA_PROFILE_AV_WMV9:
    register_profile (g_profiler, &dlna_profile_av_wmv9);
    break;
  default:
    break;
  }
}

static int
is_profile_registered (ffmpeg_profiler_data_t *data, ffmpeg_profiler_media_profile_t profile)
{
  void **p;

  p = &data->first_profile;
  while (*p)
  {
    if (((registered_profile_t *) *p)->id == profile)
      return 1;
    p = (void *) &((registered_profile_t *) *p)->next;
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

static char **
ffmpeg_profiler_get_supported_mime_types ()
{
  char **mimes;

  if (!g_profiler || !g_profiler->inited)
    return NULL;

  if (g_profiler->mimes)
    return g_profiler->mimes;
  else
    g_profiler->mimes = calloc (1, sizeof (char*));
  mimes = g_profiler->mimes;

    if (is_profile_registered (g_profiler, DLNA_PROFILE_IMAGE_JPEG))
      mimes = dlna_list_add (mimes, MIME_IMAGE_JPEG);

    if (is_profile_registered (g_profiler, DLNA_PROFILE_IMAGE_PNG))
      mimes = dlna_list_add (mimes, MIME_IMAGE_PNG);
    
    if (is_profile_registered (g_profiler, DLNA_PROFILE_AUDIO_AC3))
      mimes = dlna_list_add (mimes, MIME_AUDIO_DOLBY_DIGITAL);
    
    if (is_profile_registered (g_profiler, DLNA_PROFILE_AUDIO_AMR))
    {
      mimes = dlna_list_add (mimes, MIME_AUDIO_MPEG_4);
      mimes = dlna_list_add (mimes, MIME_AUDIO_3GP);
    }
    
    if (is_profile_registered (g_profiler, DLNA_PROFILE_AUDIO_ATRAC3))
      mimes = dlna_list_add (mimes, MIME_AUDIO_ATRAC);
    
    if (is_profile_registered (g_profiler, DLNA_PROFILE_AUDIO_LPCM))
      mimes = dlna_list_add (mimes, MIME_AUDIO_LPCM);
    
    if (is_profile_registered (g_profiler, DLNA_PROFILE_AUDIO_MP3))
      mimes = dlna_list_add (mimes, MIME_AUDIO_MPEG);
    
    if (is_profile_registered (g_profiler, DLNA_PROFILE_AUDIO_MPEG4))
    {
      mimes = dlna_list_add (mimes, MIME_AUDIO_ADTS);
      mimes = dlna_list_add (mimes, MIME_AUDIO_MPEG_4);
    }
    
    if (is_profile_registered (g_profiler, DLNA_PROFILE_AUDIO_WMA))
      mimes = dlna_list_add (mimes, MIME_AUDIO_WMA);
    
    if (is_profile_registered (g_profiler, DLNA_PROFILE_AV_MPEG1))
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG);
    
    if (is_profile_registered (g_profiler, DLNA_PROFILE_AV_MPEG2))
    {
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG);
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG_TS);
    }
    
    if (is_profile_registered (g_profiler, DLNA_PROFILE_AV_MPEG4_PART2))
    {
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG);
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG_4);
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG_TS);
    }
    
    if (is_profile_registered (g_profiler, DLNA_PROFILE_AV_MPEG4_PART10))
    {
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG);
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG_4);
      mimes = dlna_list_add (mimes, MIME_VIDEO_MPEG_TS);
    }
    
    if (is_profile_registered (g_profiler, DLNA_PROFILE_AV_WMV9))
      mimes = dlna_list_add (mimes, MIME_VIDEO_WMV);
  mimes = dlna_list_add (mimes, NULL);

  g_profiler->mimes = mimes;
  return g_profiler->mimes;
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

dlna_profile_t *
ffmpeg_profiler_get_media_profile (char *profileid)
{
  registered_profile_t *p;
  int i = 0;

  if (!g_profiler || !g_profiler->inited)
    return NULL;

  p = g_profiler->first_profile;
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
  return NULL;
}

static void
ffmpeg_profiler_free (dlna_profiler_t *profiler dlna_unused)
{
  if (g_profiler)
  {
    if (g_profiler->mimes)
      free (g_profiler->mimes);
    avformat_network_deinit ();
    free (g_profiler);
  }
}

static void
media_profile_free(dlna_item_t *item)
{
  ffmpeg_profile_t *cookie = (ffmpeg_profile_t *)item->profile_cookie;

  avformat_close_input (&cookie->ctx);
  free (cookie);
  item->profile_cookie = NULL;
}

dlna_profile_t *
ffmpeg_profiler_guess_media_profile (dlna_stream_t *reader, void **cookie)
{
  registered_profile_t *p;
  dlna_profile_t *profile = NULL;
  AVFormatContext *ctx = NULL;
  av_codecs_t *codecs;
  char check_extensions = 1;

  if (!g_profiler || !g_profiler->inited)
    return NULL;

  if (avformat_open_input (&ctx, reader->url, NULL, NULL) != 0)
  {
    return NULL;
  }

  if (avformat_find_stream_info (ctx, NULL) < 0)
  {
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

  p = g_profiler->first_profile;
  while (p)
  {
    dlna_profile_t *prof;
    
    if (check_extensions)
    {
      if (p->extensions)
      {
        /* check for valid file extension */
        if (!match_file_extension (reader->url, p->extensions))
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

  if (!profile)
    return NULL;

  profile->get_properties = item_get_properties;
  profile->get_metadata = item_get_metadata;
  profile->free = media_profile_free;
  //profile->prepare_stream = ffmpeg_prepare_stream;
  //profile->read_stream = ffmpeg_read_stream;
  //profile->seek_stream = NULL;

  ffmpeg_profile_t *fprofile = calloc (1, sizeof (ffmpeg_profile_t));
  fprofile->ctx = ctx;
  *cookie = fprofile;
  free (codecs);
  return profile;
}

static dlna_properties_t *
item_get_properties (dlna_item_t *item)
{
  ffmpeg_profile_t *cookie = (ffmpeg_profile_t *)item->profile_cookie;
  AVFormatContext *ctx = cookie->ctx;
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
item_get_metadata (dlna_item_t *item)
{
  ffmpeg_profile_t *cookie = (ffmpeg_profile_t *)item->profile_cookie;
  AVFormatContext *ctx = cookie->ctx;
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

/*
static void *
stream_thread_play (void *arg)
{
  ffmpeg_stream_t *stream = (ffmpeg_stream_t *)arg;
  printf ("\n\n start stream \n\n");
  while (1)
  {
    ithread_mutex_lock (&stream->mutex);
    ithread_cond_wait (&stream->cond,&stream->mutex);
    ithread_mutex_unlock (&stream->mutex);
    printf ("\n\n new packet \n\n");
  }
  return NULL;
}

int
stream_component_open (ffmpeg_profile_t *cookie, int stream_type)
{
  ffmpeg_stream_t *stream = &(cookie->stream[stream_type]);
  AVCodecContext *avctx;
  AVCodec *codec;
  AVDictionary *opts = NULL;

  avctx = cookie->ctx->streams[stream->id]->codec;
  codec = avcodec_find_decoder(avctx->codec_id);

  if (!codec)
    return -1;
  avctx->codec_id = codec->id;
  if (!opts || !av_dict_get(opts, "threads", NULL, 0))
    av_dict_set(&opts, "threads", "auto", 0);
  if (avctx->codec_type == AVMEDIA_TYPE_VIDEO || avctx->codec_type == AVMEDIA_TYPE_AUDIO)
    av_dict_set(&opts, "refcounted_frames", "1", 0);
  if (avcodec_open2(avctx, codec, &opts) < 0)
    return -1;

  sound_tinyalsa_open (avctx->channels, AV_SAMPLE_FMT_S16, avctx->sample_rate);
  ithread_mutex_init (&stream->mutex, NULL);
  ithread_cond_init (&stream->cond, NULL);
  ithread_create (&stream->thread, NULL, stream_thread_play, stream);
  stream->type = stream_type;

  return 0;
}

int
stream_push_packet (ffmpeg_stream_t *stream, AVPacket *pkt)
{
  if (stream->type == AVMEDIA_TYPE_AUDIO)
  {
    ret = sound_tinyalsa_write (pkt->
  return 0;
}

static int wanted_stream [AVMEDIA_TYPE_NB] = {
    [AVMEDIA_TYPE_AUDIO]    = -1,
    [AVMEDIA_TYPE_VIDEO]    = -1,
    [AVMEDIA_TYPE_SUBTITLE] = -1,
};

static int
ffmpeg_prepare_stream (dlna_item_t *item)
{
  ffmpeg_profile_t *cookie = (ffmpeg_profile_t *)item->profile_cookie;
  AVFormatContext *ctx = cookie->ctx;

  printf ("\n\n prepare frame \n\n");
  cookie->stream[AVMEDIA_TYPE_VIDEO].id =
        av_find_best_stream(ctx, AVMEDIA_TYPE_VIDEO,
                      wanted_stream[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);
  cookie->stream[AVMEDIA_TYPE_AUDIO].id =
        av_find_best_stream(ctx, AVMEDIA_TYPE_AUDIO,
                      wanted_stream[AVMEDIA_TYPE_AUDIO],
                      cookie->stream[AVMEDIA_TYPE_VIDEO].id, NULL, 0);
  cookie->stream[AVMEDIA_TYPE_SUBTITLE].id =
        av_find_best_stream(ctx, AVMEDIA_TYPE_SUBTITLE,
                      wanted_stream[AVMEDIA_TYPE_SUBTITLE],
                      (cookie->stream[AVMEDIA_TYPE_AUDIO].id >= 0 ?
                         cookie->stream[AVMEDIA_TYPE_AUDIO].id :
                         cookie->stream[AVMEDIA_TYPE_VIDEO].id), NULL, 0);
  if (cookie->stream[AVMEDIA_TYPE_AUDIO].id >= 0)
  {
    stream_component_open(cookie, AVMEDIA_TYPE_AUDIO);
  }
  return 0;
}

static int
ffmpeg_read_stream (dlna_item_t *item)
{
  ffmpeg_profile_t *cookie = (ffmpeg_profile_t *)item->profile_cookie;
  ffmpeg_stream_t *stream;
  AVFormatContext *ctx = cookie->ctx;
  AVPacket pkt1, *pkt = &pkt1;
  int ret, i;

  av_init_packet(&pkt);
  pkt.data = NULL;
  pkt.size = 0;
  ret = av_read_frame(ctx, pkt);
  printf ("\n read frame %d\n", ret);
  if (ret < 0)
    return -1;
  printf ("index %d ", pkt->stream_index);
  for (i = 0; i < AVMEDIA_TYPE_NB; i++)
  {
    stream = &cookie->stream[i];
    printf ("id %d ", stream->id);
    if (pkt->stream_index == stream->id)
    {
      stream_push_packet (stream, pkt);
      break;
    }
  }
  av_free_packet (pkt);
  printf ("\n");
  return ret;
}
*/
const dlna_profiler_t ffmpeg_profiler = {
  .init = ffmpeg_profiler_init,
  .guess_media_profile = ffmpeg_profiler_guess_media_profile,
  .get_media_profile = ffmpeg_profiler_get_media_profile,
  .get_supported_mime_types = ffmpeg_profiler_get_supported_mime_types,
  .free = ffmpeg_profiler_free,
};
