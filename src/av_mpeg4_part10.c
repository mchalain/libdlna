/*
 * libdlna: reference DLNA standards implementation.
 * Copyright (C) 2007 Benjamin Zores <ben@geexbox.org>
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <ffmpeg/avcodec.h>

#include "dlna.h"
#include "profiles.h"
#include "containers.h"

typedef enum {
  AVC_VIDEO_PROFILE_INVALID,
  AVC_VIDEO_PROFILE_BL_QCIF15,
  AVC_VIDEO_PROFILE_BL_L1B_QCIF,
  AVC_VIDEO_PROFILE_BL_L12_CIF15,
  AVC_VIDEO_PROFILE_BL_CIF15,
  AVC_VIDEO_PROFILE_BL_L2_CIF30,
  AVC_VIDEO_PROFILE_BL_CIF30,
  AVC_VIDEO_PROFILE_BL_L3L_SD,
  AVC_VIDEO_PROFILE_BL_L3_SD,
  AVC_VIDEO_PROFILE_MP_SD,
  AVC_VIDEO_PROFILE_MP_HD
} avc_video_profile_t;

typedef enum {
  AVC_AUDIO_PROFILE_INVALID,
  AVC_AUDIO_PROFILE_AMR_WB,
  AVC_AUDIO_PROFILE_AMR,
  AVC_AUDIO_PROFILE_ATRAC,
  AVC_AUDIO_PROFILE_BSAC_MULT5,
  AVC_AUDIO_PROFILE_BSAC,
  AVC_AUDIO_PROFILE_AAC_LTP_MULT7,
  AVC_AUDIO_PROFILE_AAC_LTP_MULT5,
  AVC_AUDIO_PROFILE_AAC_LTP,
  AVC_AUDIO_PROFILE_AC3,
  AVC_AUDIO_PROFILE_MP3,
  AVC_AUDIO_PROFILE_HEAAC,  
  AVC_AUDIO_PROFILE_AAC_MULT5,
  AVC_AUDIO_PROFILE_AAC
} avc_audio_profile_t;

static dlna_profile_t *
probe_avc (AVFormatContext *ctx)
{
  av_codecs_t *codecs;
  dlna_container_type_t st;

  /* grab codecs info */
  codecs = av_profile_get_codecs (ctx);
  if (!codecs)
    goto probe_avc_end;

  /* check for H.264/AVC codec */
  if (codecs->vc->codec_id != CODEC_ID_H264)
    goto probe_avc_end;

  /* check for a supported container */
  st = stream_get_container (ctx);
  if (st != CT_MOV && /* MP4 or 3GPP */
      st != CT_MPEG_TRANSPORT_STREAM &&
      st != CT_MPEG_TRANSPORT_STREAM_DLNA &&
      st != CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS)
    goto probe_avc_end;
  
 probe_avc_end:
  if (codecs)
    free (codecs);
  
  return NULL;
}

dlna_registered_profile_t dlna_profile_av_mpeg4_part10 = {
  .id = DLNA_PROFILE_AV_MPEG4_PART10,
  .extensions = "mp4,3gp,3gpp,asf,mpg,mpeg,mpe,mp2t,ts",
  .probe = probe_avc,
  .next = NULL
};
