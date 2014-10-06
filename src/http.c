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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "upnp_internals.h"
#include "services.h"
#include "vfs.h"
#include "devices.h"

#define PROTOCOL_TYPE_PRE_SZ  11   /* for the str length of "http-get:*:" */
#define PROTOCOL_TYPE_SUFF_SZ 2    /* for the str length of ":*" */

typedef struct http_file_handler_s {
  char *fullpath;
  off_t pos;
  enum {
    HTTP_FILE_LOCAL,
    HTTP_FILE_MEMORY
  } type;
  union {
    struct {
      int fd;
    } local;
    struct {
      char *content;
      off_t len;
    } memory;
  } detail;
} http_file_handler_t;

struct http_resource_s
{
  uint32_t id;
};

static char *
http_url (vfs_resource_t *resource)
{
  char *url;
  struct http_resource_s *cookie = resource->cookie;
  
  url = malloc (1024);
  sprintf (url, "http://%s:%d%s/%u",
                      dlnaGetServerIpAddress (),
                      dlnaGetServerPort (), VIRTUAL_DIR, cookie->id);
  return url;
}

static char *
http_protocol_info (vfs_resource_t *resource, dlna_org_flags_t flags)
{
  char *protocol_info;
  
  protocol_info =
    dlna_write_protocol_info (resource->info.protocolid,
                      resource->info.speed,
                      resource->info.cnv,
                      resource->info.op,
                      flags, resource->profile);
  return protocol_info;
}

static vfs_resource_t *
dlna_http_resource_new (vfs_item_t *item)
{
  vfs_resource_t *resource;
  dlna_item_t *dlna_item;
  struct http_resource_s *cookie;

  resource = calloc (1, sizeof (vfs_resource_t));
  cookie = calloc (1, sizeof (struct http_resource_s));
  cookie->id = item->id;
  resource->cookie = cookie;
  resource->url = http_url;
  resource->protocol_info = http_protocol_info;

  dlna_item = item->u.resource.item;
  resource->profile = dlna_item->profile;
  resource->size = dlna_item->filesize;
  memcpy (&resource->properties, dlna_item->properties, sizeof (dlna_properties_t));
  resource->info.protocolid = DLNA_PROTOCOL_INFO_TYPE_HTTP;
  resource->info.cnv = DLNA_ORG_CONVERSION_NONE;
  resource->info.op = DLNA_ORG_OPERATION_RANGE;
  resource->info.speed = DLNA_ORG_PLAY_SPEED_NORMAL;
  return resource;
}

dlna_protocol_t *
http_protocol_new (dlna_t *dlna)
{
  dlna_protocol_t *protocol;

  protocol = calloc (1, sizeof (dlna_protocol_t));
  protocol->type = DLNA_PROTOCOL_INFO_TYPE_HTTP;
  protocol->create_resource = dlna_http_resource_new;
}

static inline void
set_service_http_info (struct File_Info *info,
                       const size_t length,
                       const char *content_type)
{
  info->file_length   = length;
  info->last_modified = 0;
  info->is_directory  = 0;
  info->is_readable   = 1;
  info->content_type  = ixmlCloneDOMString (content_type);
}

static int
dlna_http_get_info (void *cookie,
                    const char *filename,
                    struct File_Info *info)
{
  dlna_t *dlna;
  uint32_t id;
  vfs_item_t *item;
  dlna_item_t *dlna_item;
  struct stat st;
  dlna_service_t *service;
  
  if (!cookie || !filename || !info)
    return HTTP_ERROR;

  dlna = (dlna_t *) cookie;

  dlna_log (DLNA_MSG_INFO,
            "%s, filename : %s\n", __FUNCTION__, filename);

  /* trap application-level HTTP callback */
  dlna_http_callback_t *http_callback;
  for (http_callback = dlna->http_callback; http_callback; http_callback = http_callback->next)
  {
    dlna_stream_t *stream = NULL;
    if (http_callback->open)
      stream = http_callback->open (http_callback->cookie, filename);
    if (stream)
    {
      set_service_http_info (info, stream->length, stream->mime);
      stream->close (stream);
      return HTTP_OK;
    }
  }
  
  /* look for service directory */
  if (!strncmp (filename, SERVICES_VIRTUAL_DIR, SERVICES_VIRTUAL_DIR_LEN))
  {
    /* look for the good service location */
    service = dlna_service_find_url (dlna->device, (char *)filename + SERVICES_VIRTUAL_DIR_LEN + 1);

    /* return the service description if available */
    if (service)
    {
      char *description = service->get_description (service);

      set_service_http_info (info, strlen(description), SERVICE_CONTENT_TYPE);
      free (description);
      return HTTP_OK;
    }
  }

  dlna_vfs_t *vfs = dlna_service_find_id (dlna->device, DLNA_SERVICE_CONTENT_DIRECTORY)->cookie;

  /* ask for anything else ... */
  id = strtoul (strrchr (filename, '/') + 1, NULL, 10);
  item = vfs_get_item_by_id (vfs, id);
  if (!item)
    return HTTP_ERROR;

  if (item->type != DLNA_RESOURCE)
    return HTTP_ERROR;

  dlna_item = vfs_item_get(item);
  if (!dlna_item)
    return HTTP_ERROR;

  if (!dlna_item->filename)
    return HTTP_ERROR;

  if (stat (dlna_item->filename, &st) < 0)
    return HTTP_ERROR;

  info->is_readable = 1;
  if (access (dlna_item->filename, R_OK) < 0)
  {
    if (errno != EACCES)
      return HTTP_ERROR;
    info->is_readable = 0;
  }

  /* file exist and can be read */
  info->file_length = st.st_size;
  info->last_modified = st.st_mtime;
  info->is_directory = S_ISDIR (st.st_mode);

  if (dlna_item->profile->mime)
  {
    info->content_type = ixmlCloneDOMString (dlna_item->profile->mime);
  }
  else
    info->content_type = ixmlCloneDOMString ("");
  
  return HTTP_OK;
}

static dlnaWebFileHandle
http_get_file_from_memory (const char *fullpath,
                           const char *description,
                           const size_t length)
{
  dlna_http_file_handler_t *dhdl;
  http_file_handler_t *hdl;

  if (!fullpath || !description || length == 0)
    return NULL;
  
  hdl                        = malloc (sizeof (http_file_handler_t));
  hdl->fullpath              = strdup (fullpath);
  hdl->pos                   = 0;
  hdl->type                  = HTTP_FILE_MEMORY;
  hdl->detail.memory.content = strdup (description);
  hdl->detail.memory.len     = length;

  dhdl                       = malloc (sizeof (dlna_http_file_handler_t));
  dhdl->external             = 0;
  dhdl->priv                 = hdl;
  
  return ((dlnaWebFileHandle) dhdl);
}

static dlnaWebFileHandle
http_get_file_local (dlna_item_t *dlna_item)
{
  dlna_http_file_handler_t *dhdl;
  http_file_handler_t *hdl;
  int fd;
  
  if (!dlna_item)
    return NULL;

  if (!dlna_item->filename)
    return NULL;
  
  fd = open (dlna_item->filename,
             O_RDONLY | O_NONBLOCK | O_SYNC | O_NDELAY);
  if (fd < 0)
    return NULL;
  
  hdl                        = malloc (sizeof (http_file_handler_t));
  hdl->fullpath              = strdup (dlna_item->filename);
  hdl->pos                   = 0;
  hdl->type                  = HTTP_FILE_LOCAL;
  hdl->detail.local.fd       = fd;

  dhdl                       = malloc (sizeof (dlna_http_file_handler_t));
  dhdl->external             = 0;
  dhdl->priv                 = hdl;

  return ((dlnaWebFileHandle) dhdl);
}

static dlnaWebFileHandle
dlna_http_open (void *cookie,
                const char *filename,
                enum dlnaOpenFileMode mode)
{
  dlna_t *dlna;
  uint32_t id;
  vfs_item_t *item;
  dlna_item_t *dlna_item;
  dlna_service_t *service;
  dlna_service_list_t *it;

  if (!cookie || !filename)
    return NULL;

  dlna = (dlna_t *) cookie;

  dlna_log (DLNA_MSG_INFO,
            "%s, filename : %s\n", __FUNCTION__, filename);

  if (mode != DLNA_READ)
    return NULL;

  /* trap application-level HTTP callback */
  dlna_http_callback_t *http_callback;
  for (http_callback = dlna->http_callback; http_callback; http_callback = http_callback->next)
  {
    dlna_stream_t *stream = NULL;
    if (http_callback->open)
      stream = http_callback->open (http_callback->cookie, filename);
    if (stream)
    {
      dlna_http_file_handler_t *dhdl;
      dhdl = calloc (1, sizeof (dlna_http_file_handler_t));
      dhdl->external = 1;
      dhdl->priv = stream;
      return (dlnaWebFileHandle)dhdl;
    }
  }
  
  /* look for service directory */
  if (!strncmp (filename, SERVICES_VIRTUAL_DIR, SERVICES_VIRTUAL_DIR_LEN))
  {
    /* look for the good service location */
    service = dlna_service_find_url (dlna->device, (char *)filename + SERVICES_VIRTUAL_DIR_LEN + 1);

    /* return the service description if available */
    if (service)
    {
      dlnaWebFileHandle ret;
      char *description = service->get_description (service);

      ret = http_get_file_from_memory (filename, description, strlen(description));
      free (description);
      return ret;
    }
  }

  dlna_vfs_t *vfs;
  int cdsid = DLNA_SERVICE_CONTENT_DIRECTORY;
  HASH_FIND_INT(dlna->device->services, &cdsid, it);
  vfs = (dlna_vfs_t *)it->service->cookie;

  /* ask for anything else ... */
  id = strtoul (strrchr (filename, '/') + 1, NULL, 10);
  item = vfs_get_item_by_id (vfs, id);
  if (!item)
    return NULL;

  dlna_item = vfs_item_get(item);
  if (!dlna_item)
    return NULL;
  return http_get_file_local (dlna_item);
}

static int
dlna_http_read (void *cookie,
                dlnaWebFileHandle fh,
                char *buf,
                size_t buflen)
{
  dlna_t *dlna;
  dlna_http_file_handler_t *dhdl;
  http_file_handler_t *hdl;
  ssize_t len = -1;

  if (!cookie || !fh)
    return HTTP_ERROR;

  dlna = (dlna_t *) cookie;
  dhdl = (dlna_http_file_handler_t *) fh;
  
  dlna_log (DLNA_MSG_INFO, "%s\n", __FUNCTION__);

  /* trap application-level HTTP callback */
  if (dhdl->external)
  {
    int res;
    dlna_stream_t *stream = dhdl->priv;
    if (stream->read)
      res = stream->read (stream, buf, buflen);
    if (res > 0)
      return res;
  }

  hdl = (http_file_handler_t *) dhdl->priv;
  
  switch (hdl->type)
  {
  case HTTP_FILE_LOCAL:
    dlna_log (DLNA_MSG_INFO, "Read local file.\n");
    len = read (hdl->detail.local.fd, buf, buflen);
    break;
  case HTTP_FILE_MEMORY:
    dlna_log (DLNA_MSG_INFO, "Read file from memory.\n");
    len = (ssize_t) MIN ((ssize_t)buflen, hdl->detail.memory.len - hdl->pos);
    memcpy (buf, hdl->detail.memory.content + hdl->pos, (ssize_t) len);
    break;
  default:
    dlna_log (DLNA_MSG_ERROR, "Unknown HTTP file type.\n");
    break;
  }

  if (len > 0)
    hdl->pos += len;

  dlna_log (DLNA_MSG_INFO, "Read %zd bytes.\n", len);

  return len;
}

static int
dlna_http_write (void *cookie,
                 dlnaWebFileHandle fh,
                 char *buf,
                 size_t buflen)
{
  dlna_t *dlna;
  dlna_http_file_handler_t *dhdl;

  dlna = (dlna_t *) cookie;
  dhdl = (dlna_http_file_handler_t *) fh;
  
  /* trap application-level HTTP callback */
  if (dhdl->external)
  {
    int res;
    dlna_stream_t *stream = dhdl->priv;
    if (stream->write)
      res = stream->write (stream, buf, buflen);
    if (res > 0)
      return res;
  }
  
  return 0;
}

static int
dlna_http_seek (void *cookie,
                dlnaWebFileHandle fh,
                off_t offset,
                int origin)
{
  dlna_t *dlna;
  dlna_http_file_handler_t *dhdl;
  http_file_handler_t *hdl;
  off_t newpos = -1;
  
  if (!cookie || !fh)
    return HTTP_ERROR;

  dlna = (dlna_t *) cookie;
  dhdl = (dlna_http_file_handler_t *) fh;
  
  dlna_log (DLNA_MSG_INFO, "%s\n", __FUNCTION__);

  /* trap application-level HTTP callback */
  if (dhdl->external)
  {
    int res;
    dlna_stream_t *stream = dhdl->priv;
    if (stream->lseek)
      res = stream->lseek (stream, offset, origin);
    if (res > 0)
      return res;
  }

  hdl = (http_file_handler_t *) dhdl->priv;
  
  switch (origin)
  {
  case SEEK_SET:
    dlna_log (DLNA_MSG_INFO,
              "Attempting to seek to %lld (was at %lld) in %s\n",
              offset, hdl->pos, hdl->fullpath);
    newpos = offset;
    break;
  case SEEK_CUR:
    dlna_log (DLNA_MSG_INFO,
              "Attempting to seek by %lld from %lld in %s\n",
              offset, hdl->pos, hdl->fullpath);
    newpos = hdl->pos + offset;
    break;
  case SEEK_END:
    dlna_log (DLNA_MSG_INFO,
              "Attempting to seek by %lld from end (was at %lld) in %s\n",
              offset, hdl->pos, hdl->fullpath);

    if (hdl->type == HTTP_FILE_LOCAL)
    {
      struct stat sb;
      if (stat (hdl->fullpath, &sb) < 0)
      {
        dlna_log (DLNA_MSG_ERROR,
                  "%s: cannot stat: %s\n", hdl->fullpath, strerror (errno));
        return HTTP_ERROR;
      }
      newpos = sb.st_size + offset;
    }
    else if (hdl->type == HTTP_FILE_MEMORY)
      newpos = hdl->detail.memory.len + offset;
    break;
  }

  switch (hdl->type)
  {
  case HTTP_FILE_LOCAL:
    /* Just make sure we cannot seek before start of file. */
    if (newpos < 0)
    {
      dlna_log (DLNA_MSG_ERROR,
                "%s: cannot seek: %s\n", hdl->fullpath, strerror (EINVAL));
      return HTTP_ERROR;
    }

    /* Don't seek with origin as specified above, as file may have
       changed in size since our last stat. */
    if (lseek (hdl->detail.local.fd, newpos, SEEK_SET) == -1)
    {
      dlna_log (DLNA_MSG_ERROR,
                "%s: cannot seek: %s\n", hdl->fullpath, strerror (errno));
      return HTTP_ERROR;
    }
    break;
  case HTTP_FILE_MEMORY:
    if (newpos < 0 || newpos > hdl->detail.memory.len)
    {
      dlna_log (DLNA_MSG_ERROR,
                "%s: cannot seek: %s\n", hdl->fullpath, strerror (EINVAL));
      return HTTP_ERROR;
    }
    break;
  }

  /* everything went well ... */
  hdl->pos = newpos;
  return HTTP_OK;
}

static int
dlna_http_close (void *cookie,
                 dlnaWebFileHandle fh)
{
  dlna_t *dlna;
  dlna_http_file_handler_t *dhdl;
  http_file_handler_t *hdl;
  
  if (!cookie || !fh)
    return HTTP_ERROR;

  dlna = (dlna_t *) cookie;
  dhdl = (dlna_http_file_handler_t *) fh;
  
  dlna_log (DLNA_MSG_INFO, "%s\n", __FUNCTION__);

  /* trap application-level HTTP callback */
  if (dhdl->external)
  {
    dlna_stream_t *stream = dhdl->priv;
    if (stream->close)
      stream->close (stream);
    free (dhdl);
    return 0;
  }

  hdl = (http_file_handler_t *) dhdl->priv;
  
  switch (hdl->type)
  {
  case HTTP_FILE_LOCAL:
    close (hdl->detail.local.fd);
    break;
  case HTTP_FILE_MEMORY:
    /* no close operation is needed, just free file content */
    if (hdl->detail.memory.content)
      free (hdl->detail.memory.content);
    break;
  default:
    dlna_log (DLNA_MSG_ERROR, "Unknown HTTP file type.\n");
    break;
  }

  if (hdl->fullpath)
    free (hdl->fullpath);
  free (hdl);
  free (dhdl);

  return HTTP_OK;
}

#ifndef HAVE_EXTERNAL_LIBUPNP
struct dlnaVirtualDirCallbacks virtual_dir_callbacks = {
  .cookie = NULL,
  .get_info = dlna_http_get_info,
  .open = dlna_http_open,
  .read = dlna_http_read,
  .write = dlna_http_write,
  .seek = dlna_http_seek,
  .close = dlna_http_close
};
#else
static void *http_cookie;

int dlnaSetVirtualDirCallbacks(
    struct dlnaVirtualDirCallbacks *callbacks,
    void *cookie)
{
  http_cookie = cookie;
  return UpnpSetVirtualDirCallbacks(callbacks);
}

static int
upnp_http_get_info (
                    const char *filename,
                    struct File_Info *info)
{
  return dlna_http_get_info(http_cookie, filename, info);
}

static dlnaWebFileHandle
upnp_http_open (
                const char *filename,
                enum dlnaOpenFileMode mode)
{
  return dlna_http_open(http_cookie, filename, mode);
}

static int
upnp_http_read (
                 dlnaWebFileHandle fh,
                 char *buf,
                 size_t buflen)
{
  return dlna_http_read(http_cookie, fh, buf, buflen);
}

static int
upnp_http_write (
                 dlnaWebFileHandle fh,
                 char *buf,
                 size_t buflen)
{
  return dlna_http_write(http_cookie, fh, buf, buflen);
}

static int
upnp_http_seek (
                dlnaWebFileHandle fh,
                off_t offset,
                int origin)
{
  return dlna_http_seek(http_cookie, fh, offset, origin);
}

static int
upnp_http_close (dlnaWebFileHandle fh)
{
  return dlna_http_close(http_cookie, fh);
}

struct UpnpVirtualDirCallbacks virtual_dir_callbacks = {
  .get_info = upnp_http_get_info,
  .open = upnp_http_open,
  .read = upnp_http_read,
  .write = upnp_http_write,
  .seek = upnp_http_seek,
  .close = upnp_http_close
};
#endif
