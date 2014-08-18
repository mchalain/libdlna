#ifndef FFMPEG_PROFILER_H
#define FFMPEG_PROFILER_H

dlna_profile_t *
dlna_guess_media_profile (dlna_t *dlna, dlna_item_t *item);
char **
ffmpeg_profiler_get_supported_mime_types (dlna_t *dlna, char **mimes);
dlna_profile_t *
ffmpeg_profiler_get_media_profile (dlna_t *dlna, char *profileid);

#endif
