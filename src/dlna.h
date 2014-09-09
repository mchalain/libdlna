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

#ifndef DLNA_H
#define DLNA_H

/**
 * @file dlna.h
 * external api header.
 */

#ifdef __cplusplus
extern "C" {
#if 0 /* avoid EMACS indent */
}
#endif /* 0 */
#endif /* __cplusplus */

#include <inttypes.h>
#include <sys/types.h>

#define DLNA_STRINGIFY(s)         DLNA_TOSTRING(s)
#define DLNA_TOSTRING(s) #s

#define LIBDLNA_VERSION_INT  ((0<<16)+(3<<8)+0)
#define LIBDLNA_VERSION      0.3.0
#define LIBDLNA_BUILD        LIBDLNA_VERSION_INT

#define LIBDLNA_IDENT        "DLNA " DLNA_STRINGIFY(LIBDLNA_VERSION)

/* DLNA MIME types */
#define MIME_IMAGE_JPEG                   "image/jpeg"
#define MIME_IMAGE_PNG                    "image/png"
#define MIME_IMAGE_BMP                    "image/bmp"
#define MIME_IMAGE_GIF                    "image/gif"
#define MIME_IMAGE_TIFF                   "image/tiff"
#define MIME_IMAGE_QT                     "image/x-quicktime"

#define MIME_AUDIO_3GP                    "audio/3gpp"
#define MIME_AUDIO_ADTS                   "audio/vnd.dlna.adts"
#define MIME_AUDIO_ATRAC                  "audio/x-sony-oma"
#define MIME_AUDIO_DOLBY_DIGITAL          "audio/vnd.dolby.dd-raw"
#define MIME_AUDIO_LPCM                   "audio/L16"
#define MIME_AUDIO_MPEG                   "audio/mpeg"
#define MIME_AUDIO_MPEG_4                 "audio/mp4"
#define MIME_AUDIO_WMA                    "audio/x-ms-wma"
#define MIME_AUDIO_AAC                    "audio/x-aac"
#define MIME_AUDIO_AC3                    "audio/x-ac3"
#define MIME_AUDIO_AIF                    "audio/aiff"
#define MIME_AUDIO_OGG                    "audio/x-ogg"
#define MIME_AUDIO_WAV                    "audio/wav"
#define MIME_AUDIO_FLAC                   "audio/x-flac"
#define MIME_AUDIO_MIDI                   "audio/midi"
#define MIME_AUDIO_REAL                   "audio/x-pn-realaudio"

#define MIME_VIDEO_3GP                    "video/3gpp"
#define MIME_VIDEO_ASF                    "video/x-ms-asf"
#define MIME_VIDEO_MPEG                   "video/mpeg"
#define MIME_VIDEO_MPEG_2                 "video/mpeg2"
#define MIME_VIDEO_MPEG_4                 "video/mp4"
#define MIME_VIDEO_MPEG_TS                "video/vnd.dlna.mpeg-tts"
#define MIME_VIDEO_WMV                    "video/x-ms-wmv"
#define MIME_VIDEO_AVI                    "video/avi"
#define MIME_VIDEO_QT                     "video/quicktime"

/* DLNA Labels */
#define LABEL_IMAGE_PICTURE               "picture"
#define LABEL_IMAGE_ICON                  "icon"

#define LABEL_AUDIO_MONO                  "mono"
#define LABEL_AUDIO_2CH                   "2-ch"
#define LABEL_AUDIO_2CH_MULTI             "2-ch multi"
#define LABEL_AUDIO_MULTI                 "multi"

#define LABEL_VIDEO_CIF15                 "CIF15"
#define LABEL_VIDEO_CIF30                 "CIF30"
#define LABEL_VIDEO_QCIF15                "QCIF15"
#define LABEL_VIDEO_SD                    "SD"
#define LABEL_VIDEO_HD                    "HD"

/***************************************************************************/
/*                                                                         */
/* DLNA Library Common Utilities                                           */
/*  Mandatory: Used to configure the whole instance of the library.        */
/*                                                                         */
/***************************************************************************/
struct dlna_metadata_s;
struct dlna_properties_s;
struct dlna_item_s;

typedef struct dlna_item_s dlna_item_t;

/* Status code for DLNA related functions */
typedef enum {
  DLNA_ST_OK,
  DLNA_ST_ERROR,
  DLNA_ST_VFSEMPTY
} dlna_status_code_t;

/* Verbosity level: defines which kind of log can be displayed */
typedef enum {
  DLNA_MSG_NONE,          /* no error messages */
  DLNA_MSG_INFO,          /* working operations */
  DLNA_MSG_WARNING,       /* harmless failures */
  DLNA_MSG_ERROR,         /* may result in hazardous behavior */
  DLNA_MSG_CRITICAL,      /* prevents lib from working */
} dlna_verbosity_level_t;

/* DLNA Capability/Compatibility mode settings */
typedef enum {
  DLNA_CAPABILITY_DLNA,          /* comply with DLNA specifications */
  DLNA_CAPABILITY_UPNP_AV,       /* comply with UPnP A/V specifications */
  DLNA_CAPABILITY_UPNP_AV_XBOX,  /* UPnP A/V with XboX 360 hacks */
} dlna_capability_mode_t;

typedef enum {
  DLNA_PROTOCOL_INFO_TYPE_UNKNOWN,
  DLNA_PROTOCOL_INFO_TYPE_HTTP,
  DLNA_PROTOCOL_INFO_TYPE_RTP,
  DLNA_PROTOCOL_INFO_TYPE_ANY
} dlna_protocol_info_type_t;

/* DLNA.ORG_PS: play speed parameter (integer)
 *     0 invalid play speed
 *     1 normal play speed
 */
typedef enum {
  DLNA_ORG_PLAY_SPEED_INVALID = 0,
  DLNA_ORG_PLAY_SPEED_NORMAL = 1,
} dlna_org_play_speed_t;

/* DLNA.ORG_CI: conversion indicator parameter (integer)
 *     0 not transcoded
 *     1 transcoded
 */
typedef enum {
  DLNA_ORG_CONVERSION_NONE = 0,
  DLNA_ORG_CONVERSION_TRANSCODED = 1,
} dlna_org_conversion_t;

/* DLNA.ORG_OP: operations parameter (string)
 *     "00" (or "0") neither time seek range nor range supported
 *     "01" range supported
 *     "10" time seek range supported
 *     "11" both time seek range and range supported
 */
typedef enum {
  DLNA_ORG_OPERATION_NONE                  = 0x00,
  DLNA_ORG_OPERATION_RANGE                 = 0x01,
  DLNA_ORG_OPERATION_TIMESEEK              = 0x10,
} dlna_org_operation_t;

/* DLNA.ORG_FLAGS, padded with 24 trailing 0s
 *     80000000  31  senderPaced
 *     40000000  30  lsopTimeBasedSeekSupported
 *     20000000  29  lsopByteBasedSeekSupported
 *     10000000  28  playcontainerSupported
 *      8000000  27  s0IncreasingSupported
 *      4000000  26  sNIncreasingSupported
 *      2000000  25  rtspPauseSupported
 *      1000000  24  streamingTransferModeSupported
 *       800000  23  interactiveTransferModeSupported
 *       400000  22  backgroundTransferModeSupported
 *       200000  21  connectionStallingSupported
 *       100000  20  dlnaVersion15Supported
 *
 *     Example: (1 << 24) | (1 << 22) | (1 << 21) | (1 << 20)
 *       DLNA.ORG_FLAGS=01700000[000000000000000000000000] // [] show padding
 */
typedef enum {
  DLNA_ORG_FLAG_SENDER_PACED               = (1 << 31),
  DLNA_ORG_FLAG_TIME_BASED_SEEK            = (1 << 30),
  DLNA_ORG_FLAG_BYTE_BASED_SEEK            = (1 << 29),
  DLNA_ORG_FLAG_PLAY_CONTAINER             = (1 << 28),
  DLNA_ORG_FLAG_S0_INCREASE                = (1 << 27),
  DLNA_ORG_FLAG_SN_INCREASE                = (1 << 26),
  DLNA_ORG_FLAG_RTSP_PAUSE                 = (1 << 25),
  DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE    = (1 << 24),
  DLNA_ORG_FLAG_INTERACTIVE_TRANSFERT_MODE = (1 << 23),
  DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE  = (1 << 22),
  DLNA_ORG_FLAG_CONNECTION_STALL           = (1 << 21),
  DLNA_ORG_FLAG_DLNA_V15                   = (1 << 20),
} dlna_org_flags_t;

/**
 * DLNA Library's controller.
 * This controls the whole library.
 */
typedef struct dlna_s dlna_t;

/**
 * DLNA Device's controller.
 */
typedef struct dlna_device_s dlna_device_t;

/**
 * Initialization of library.
 *
 * @warning This function must be called before any libdlna function.
 * @return DLNA library's controller.
 */
dlna_t *dlna_init (void);

/**
 * Uninitialization of library.
 *
 * @param[in] dlna The DLNA library's controller.
 */
void dlna_uninit (dlna_t *dlna);

/**
 * Start device subscription, webserver and all internals composants.
 *
 * @param[in] dlna  The DLNA library's controller.
 *
  * @return   DLNA_ST_OK in case of success, DLNA_ST_ERROR otherwise.
 */
int dlna_start (dlna_t *dlna);

/**
 * Stop device registration, webserver and all internals composants.
 *
 * @param[in] dlna  The DLNA library's controller.
 *
  * @return   DLNA_ST_OK in case of success, DLNA_ST_ERROR otherwise.
 */
int dlna_stop (dlna_t *dlna);

/**
 * Set library's verbosity level.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] level Level of verbosity
 */
void dlna_set_verbosity (dlna_t *dlna, dlna_verbosity_level_t level);

/**
 * Set library's capability/compatibility mode.
 *  N.B. Very few devices actualyl support DLNA specifications.
 *       If you have any problem having your device regonized,
 *       just downgrade to UPnP A/V compatibility mode which is just
 *       a non-restricted mode of DLNA specifications.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] mode  Capability mode
 */
void dlna_set_capability_mode (dlna_t *dlna, dlna_capability_mode_t mode);

/**
 * Set library's mask of flags.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] flags Mask of flags to be set
 */
void dlna_set_org_flags (dlna_t *dlna, dlna_org_flags_t flags);

/**
 * Set library's check level on files extension.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] level Level of check (0 for no check, 1 to enable checks).
 */
void dlna_set_extension_check (dlna_t *dlna, int level);

/**
 * Set library's network interface to use.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] itf   Name of the interface to be used.
 */
void dlna_set_interface (dlna_t *dlna, char *itf);

/**
 * Set library's network port to use (if available).
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] port  The port number.
 */
void dlna_set_port (dlna_t *dlna, int port);

/**
 * Set the main UPnP Device
 * 
 * The device is automaticly freed when it change.
 * 
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] device  The device.
 */
void dlna_set_device (dlna_t *dlna, dlna_device_t *device);

/***************************************************************************/
/*                                                                         */
/* DLNA Item Profile Handling                                              */
/*  Optional: Used to figure out which DLNA profile a file complies with.  */
/*                                                                         */
/***************************************************************************/

typedef enum {
  DLNA_CLASS_UNKNOWN,
  DLNA_CLASS_IMAGE,
  DLNA_CLASS_AUDIO,
  DLNA_CLASS_AV,
  DLNA_CLASS_COLLECTION
} dlna_media_class_t;

/**
 * DLNA profile.
 * This specifies the DLNA profile one file/stream is compatible with.
 */
typedef struct dlna_profile_s {
  /* Profile ID, part of DLNA.ORG_PN= string */
  const char *id;
  /* Profile MIME type */
  const char *mime;
  /* Profile Label */
  const char *label;
  /* Profile type: IMAGE / AUDIO / AV */
  dlna_media_class_t media_class;
  /* properties extraction callback */
  struct dlna_properties_s *(*get_properties)(struct dlna_item_s *item);
  struct dlna_metadata_s *(*get_metadata)(struct dlna_item_s *item);
  void (*free)(struct dlna_item_s *item);
  int (*prepare_stream) (struct dlna_item_s *item);
  int (*read_stream) (struct dlna_item_s *item);
  void (*close_stream) (struct dlna_item_s *item);

} dlna_profile_t;

/**
 * Provides UPnP A/V ContentDirectory Object Item associated to profile.
 *
 * @warning This function returns a pointer, do _NOT_ free it.
 * @param[in] profile The DLNA profile that was targeted.
 * @return A pointer on CDS Object Item string.
 */
char *dlna_profile_upnp_object_item (dlna_profile_t *profile);

/**
 * Output the protocol information string that must be send by a DMS to a DMP
 * for the file to be played/recognized.
 *
 * @param[in] dlna    The DLNA library's controller.
 * @param[in] type    Streaming method.
 * @param[in] speed   DLNA.ORG_PS parameter.
 * @param[in] ci      DLNA.ORG_CI parameter.
 * @param[in] op      DLNA.ORG_OP parameter.
 * @param[in] flags   DLNA.ORG_FLAGS parameter.
 * @param[in] profile The DLNA's file profile that has been guessed.
 * @return            The protocol information string.
 */
char * dlna_write_protocol_info (dlna_t *dlna, 
                                 dlna_protocol_info_type_t type,
                                 dlna_org_play_speed_t speed,
                                 dlna_org_conversion_t ci,
                                 dlna_org_operation_t op,
                                 dlna_org_flags_t flags,
                                 dlna_profile_t *p);

typedef struct dlna_profiler_s {
/**
 * Output the table of mime type supported by the profiler.
 * reentrance function
 *
 * @param[in] mimes    The table to append.
 * @return            The table with the new values added.
 */
  char **(*get_supported_mime_types) ( char **mimes);
/**
 * Output the Media profile of a type.
 *
 * @param[in] profileid    The id string of a profile.
 * @return            The profile of the file.
 */
  dlna_profile_t *(*get_media_profile) (char *profileid);
/**
 * Output the Media profile of a file.
 *
 * @param[in] filename    The file name or the URI to check.
 * @param[out] cookie     The data to set into the dlna_item_t (profile_cookie).
 * @return            The profile of the file.
 */
  dlna_profile_t *(*guess_media_profile) (char *filename, void **cookie);
/**
 * free data used by the profiler
 **/
 void (*free) ();
} dlna_profiler_t;

/**
 * Set the profiler
 * 
 * @param[in] dlna     The DLNA library's controller.
 * @param[in] profiler The Media file profiler.
 **/
void
dlna_set_profiler (dlna_t *dlna, dlna_profiler_t *profiler);

/***************************************************************************/
/*                                                                         */
/* DLNA UPnP Digital Media Server (DMS) Management                         */
/*  Mandatory: Configure the device to act as a Media Server.              */
/*                                                                         */
/***************************************************************************/

typedef enum {
  DLNA_DMS_STORAGE_MEMORY,
  DLNA_DMS_STORAGE_DB,
} dlna_dms_storage_type_t;

/**
 * Defines DMS storage type for VFS Metadata.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] type  The VFS storage type.
 * @param[in] data  Optional cookie depending on storage type:
 *                   - May be NULL for memory storage.
 *                   - Path to databased file for DB storage.
 */
void dlna_dms_set_vfs_storage_type (dlna_t *dlna,
                                    dlna_dms_storage_type_t type, char *data);

/***************************************************************************/
/*                                                                         */
/* DLNA UPnP Device Management                                             */
/*  Optional: Used to overload default device parameters.                  */
/*                                                                         */
/***************************************************************************/
#define DLNA_DEVICE_TYPE_DMS "urn:schemas-upnp-org:device:MediaServer:1"
#define DLNA_DEVICE_TYPE_DMR "urn:schemas-upnp-org:device:MediaRenderer:1"

/**
 * create a new device controller
 * 
 **/
dlna_device_t *dlna_device_new ();

/**
 * Set device UPnP device Type URN.
 *
 * @param[in] device  The UPnP device controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_type (dlna_device_t *device, char *str, char *short_str);

/**
 * Set device UPnP friendly name.
 *
 * @param[in] device  The UPnP device controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_friendly_name (dlna_device_t *device, char *str);

/**
 * Set device UPnP manufacturer name.
 *
 * @param[in] device  The UPnP device controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_manufacturer (dlna_device_t *device, char *str);

/**
 * Set device UPnP manufacturer URL.
 *
 * @param[in] device  The UPnP device controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_manufacturer_url (dlna_device_t *device, char *str);

/**
 * Set device UPnP model description.
 *
 * @param[in] device  The UPnP device controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_model_description (dlna_device_t *device, char *str);

/**
 * Set device UPnP model name.
 *
 * @param[in] device  The UPnP device controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_model_name (dlna_device_t *device, char *str);

/**
 * Set device UPnP model number.
 *
 * @param[in] device  The UPnP device controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_model_number (dlna_device_t *dlna, char *str);

/**
 * Set device UPnP model URL.
 *
 * @param[in] device  The UPnP device controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_model_url (dlna_device_t *device, char *str);

/**
 * Set device UPnP serial number.
 *
 * @param[in] device  The UPnP device controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_serial_number (dlna_device_t *device, char *str);

/**
 * Set device UPnP UUID.
 *
 * @param[in] device  The UPnP device controller.
 * @param[in] str   Value to be set ("uuid:" is automatically preprended).
 */
void dlna_device_set_uuid (dlna_device_t *device, char *str);

/**
 * Set device UPnP presentation URL.
 *
 * @param[in] device  The UPnP device controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_presentation_url (dlna_device_t *dlna, char *str);

/***************************************************************************/
/*                                                                         */
/* DLNA Services Management                                                */
/*  Optional: Used to register common services or add new ones.            */
/*                                                                         */
/***************************************************************************/

typedef struct dlna_service_s         dlna_service_t;

typedef enum {
  DLNA_SERVICE_CONNECTION_MANAGER,
  DLNA_SERVICE_CONTENT_DIRECTORY,
  DLNA_SERVICE_AV_TRANSPORT,
  DLNA_SERVICE_MS_REGISTAR,
  DLNA_SERVICE_RENDERING_CONTROL,
} dlna_service_type_t;

/**
 * Register a known DLNA/UPnP Service to be used by the device.
 *   This is automatically done for all mandatory services at device init.
 *
 * @param[in] device  The UPnP device controller.
 * @param[in] srv   The service type to be registered.
 */
void dlna_service_register (dlna_device_t *device, dlna_service_t *srv);

extern dlna_service_t *cms_service_new (dlna_t*dlna);
extern dlna_service_t *cds_service_new (dlna_t*dlna);
extern dlna_service_t *rcs_service_new (dlna_t*dlna);
extern dlna_service_t *avts_service_new (dlna_t*dlna);
extern dlna_service_t *msr_service_new (dlna_t*dlna);
/***************************************************************************/
/*                                                                         */
/* DLNA UPnP Virtual File System (VFS) Management                          */
/*  Optional: Routines to add/remove element from VFS.                     */
/*                                                                         */
/***************************************************************************/
/**
 * Create a new DLNA media object item.
 *
 * @param[in] dlna     The DLNA library's controller.
 * @param[in] filename The input file to be added.
 * @return A new DLNA object item if compatible, NULL otherwise.
 */
dlna_item_t *dlna_item_new (dlna_t *dlna, dlna_profiler_t *profiler, const char *filename);

/**
 * Free an existing DLNA media object item.
 *
 * @param[in] item     The DLNA object item to be freed.
 */
void dlna_item_free (dlna_item_t *item);

/**
 * Add a new container to the VFS layer.
 *
 * @param[in] dlna         The DLNA library's controller.
 * @param[in] name         Displayed name of the container.
 * @param[in] object_id    Expected UPnP object ID.
 * @param[in] container_id UPnP object ID of its parent.
 * @return The attrbiuted UPnP object ID if successfull, 0 otherwise.
 */
uint32_t dlna_vfs_add_container (dlna_t *dlna, char *name,
                                 uint32_t object_id, uint32_t container_id);

/**
 * Add a new resource to the VFS layer.
 *
 * @param[in] dlna         The DLNA library's controller.
 * @param[in] name         Displayed name of the resource.
 * @param[in] dlna_item    The resource created by dlna_new_item ().
 * @param[in] container_id UPnP object ID of its parent.
 * @return The attrbiuted UPnP object ID if successfull, 0 otherwise.
 */
uint32_t dlna_vfs_add_resource (dlna_t *dlna, char *name,
                                dlna_item_t *dlna_item, uint32_t container_id);

/**
 * Remove an existing item (and all its children) from VFS layer by ID.
 *
 * @param[in] dlna         The DLNA library's controller.
 * @param[in] id           Unique ID of the item to be removed.
 */
void dlna_vfs_remove_item_by_id (dlna_t *dlna, uint32_t id);

/**
 * Remove an existing item (and all its children) from VFS layer by name.
 *
 * @param[in] dlna         The DLNA library's controller.
 * @param[in] name         Name of the item to be removed.
 */
void dlna_vfs_remove_item_by_name (dlna_t *dlna, char *name);

/***************************************************************************/
/*                                                                         */
/* DLNA WebServer Callbacks & Handlers                                     */
/*  Optional: Used to overload the internal HTTP server behavior.          */
/*                                                                         */
/***************************************************************************/

/**
 * DLNA Internal WebServer File Handler
 */
typedef struct dlna_http_file_handler_s {
  int external;                   /* determines whether the file has to be
                                     handled internally by libdlna or by
                                     external application */
  void *priv;                     /* private file handler */
} dlna_http_file_handler_t;

/**
 * DLNA Internal WebServer File Information
 */
typedef struct dlna_http_file_info_s {
  off_t file_length;
  char *content_type;
} dlna_http_file_info_t;

/**
 * DLNA Internal WebServer Operation Callbacks
 *  Return 0 for success, 1 otherwise.
 */
typedef struct dlna_http_callback_s {
  int (*get_info) (const char *filename, dlna_http_file_info_t *info);
  dlna_http_file_handler_t * (*open) (const char *filename);
  int (*read) (void *hdl, char *buf, size_t len);
  int (*write) (void *hdl, char *buf, size_t len);
  int (*seek) (void *hdl, off_t offset, int origin);
  int (*close) (void *hdl);
} dlna_http_callback_t;

/**
 * Set library's WebServer Callback routines.
 *   This is used by application to overload default's HTTP routines.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] cb    Structure with HTTP callbacks.
 */
void dlna_set_http_callback (dlna_t *dlna, dlna_http_callback_t *cb);

#ifdef __cplusplus
#if 0 /* avoid EMACS indent */
{
#endif /* 0 */
}
#endif /* __cplusplus */

#endif /* DLNA_H */
