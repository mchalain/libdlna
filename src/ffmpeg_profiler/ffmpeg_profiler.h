#ifndef FFMPEG_PROFILER_H
#define FFMPEG_PROFILER_H

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
dlna_profile_t *
ffmpeg_profiler_guess_media_profile (dlna_t *dlna, char *filename, void **cookie);
char **
ffmpeg_profiler_get_supported_mime_types ( char **mimes);
dlna_profile_t *
ffmpeg_profiler_get_media_profile (char *profileid);

#endif
