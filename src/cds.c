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

/*
 * ContentDirectory service specifications can be found at:
 * http://upnp.org/standardizeddcps/documents/ContentDirectory1.0.pdf
 * http://upnp.org/specs/av/UPnP-av-ContentDirectory-v2-Service-20060531.pdf
 */

#include <stdlib.h>
#include <stdio.h>

#include "upnp_internals.h"
#include "services.h"
#include "vfs.h"
#include "cds.h"
#include "minmax.h"
#include "didl.h"

#define CDS_ARG_BROWSE_FLAG_ALLOWED \
"      <allowedValueList>" \
"        <allowedValue>BrowseMetadata</allowedValue>" \
"        <allowedValue>BrowseDirectChildren</allowedValue>" \
"      </allowedValueList>"
#define CDS_ARG_TRANSFERT_STATUS_ALLOWED \
"      <allowedValueList>" \
"        <allowedValue>COMPLETED</allowedValue>" \
"        <allowedValue>ERROR</allowedValue>" \
"        <allowedValue>IN_PROGRESS</allowedValue>" \
"        <allowedValue>STOPPED</allowedValue>" \
"      </allowedValueList>" )\

/* CDS Action Names */
#define CDS_ACTION_SEARCH_CAPS        "GetSearchCapabilities"
#define CDS_ACTION_SORT_CAPS          "GetSortCapabilities"
#define CDS_ACTION_UPDATE_ID          "GetSystemUpdateID"
#define CDS_ACTION_BROWSE             "Browse"
#define CDS_ACTION_SEARCH             "Search"
#define CDS_ACTION_CREATE_OBJ         "CreateObject"
#define CDS_ACTION_DESTROY_OBJ        "DestroyObject"
#define CDS_ACTION_UPDATE_OBJ         "UpdateObject"
#define CDS_ACTION_IMPORT_RES         "ImportResource"
#define CDS_ACTION_EXPORT_RES         "ExportResource"
#define CDS_ACTION_STOP_TRANSFER      "StopTransferResource"
#define CDS_ACTION_GET_PROGRESS       "GetTransferProgress"
#define CDS_ACTION_DELETE_RES         "DeleteResource"
#define CDS_ACTION_CREATE_REF         "CreateReference"

/* CDS Arguments */
#define CDS_ARG_SEARCH_CAPS           "SearchCaps"
#define CDS_ARG_SORT_CAPS             "SortCaps"
#define CDS_ARG_ID                    "Id"

/* CDS Browse Input Arguments */
#define CDS_ARG_OBJECT_ID             "ObjectID"
#define CDS_ARG_BROWSE_FLAG           "BrowseFlag"
#define CDS_ARG_FILTER                "Filter"
#define CDS_ARG_START_INDEX           "StartingIndex"
#define CDS_ARG_REQUEST_COUNT         "RequestedCount"
#define CDS_ARG_SORT_CRIT             "SortCriteria"
#define CDS_ARG_SEARCH_CRIT           "SearchCriteria"
#define CDS_ARG_RESULT                "Result"
#define CDS_ARG_NUM_RETURNED          "NumberReturned"
#define CDS_ARG_TOTAL_MATCHES         "TotalMatches"
#define CDS_ARG_UPDATE_ID             "UpdateID"

#define CDS_ARG_CONTAINER_ID          "ContainerID"

/* CDS Argument Values */
#define CDS_ROOT_OBJECT_ID            "0"
#define CDS_BROWSE_METADATA           "BrowseMetadata"
#define CDS_BROWSE_CHILDREN           "BrowseDirectChildren"
#define CDS_OBJECT_CONTAINER          "object.container.storageFolder"
#define CDS_DIDL_RESULT               "Result"
#define CDS_DIDL_NUM_RETURNED         "NumberReturned"
#define CDS_DIDL_TOTAL_MATCH          "TotalMatches"
#define CDS_DIDL_UPDATE_ID            "UpdateID"

/* CDS Search Parameters */
#define SEARCH_CLASS_MATCH_KEYWORD            "(upnp:class = \""
#define SEARCH_CLASS_DERIVED_KEYWORD          "(upnp:class derivedfrom \""
#define SEARCH_PROTOCOL_CONTAINS_KEYWORD      "(res@protocolInfo contains \""
#define SEARCH_OBJECT_KEYWORD                 "object"
#define SEARCH_AND                            ") and ("

/* CDS Error Codes */
#define CDS_ERR_INVALID_ACTION                401
#define CDS_ERR_INVALID_ARGS                  402
#define CDS_ERR_INVALID_VAR                   404
#define CDS_ERR_ACTION_FAILED                 501
#define CDS_ERR_INVALID_OBJECT_ID             701
#define CDS_ERR_INVALID_CURRENT_TAG_VALUE     702
#define CDS_ERR_INVALID_NEW_TAG_VALUE         703
#define CDS_ERR_REQUIRED_TAG                  704
#define CDS_ERR_READ_ONLY_TAG                 705
#define CDS_ERR_PARAMETER_MISMATCH            706
#define CDS_ERR_INVALID_SEARCH_CRITERIA       708
#define CDS_ERR_INVALID_SORT_CRITERIA         709
#define CDS_ERR_INVALID_CONTAINER             710
#define CDS_ERR_RESTRICTED_OBJECT             711
#define CDS_ERR_BAD_METADATA                  712
#define CDS_ERR_RESTRICTED_PARENT             713
#define CDS_ERR_INVALID_SOURCE                714
#define CDS_ERR_ACCESS_DENIED_SOURCE          715
#define CDS_ERR_TRANSFER_BUSY                 716
#define CDS_ERR_INVALID_TRANSFER_ID           717
#define CDS_ERR_INVALID_DESTINATION           718
#define CDS_ERR_ACESS_DENIED_DESTINATION      719
#define CDS_ERR_PROCESS_REQUEST               720

/*
 * GetSearchCapabilities:
 *   This action returns the searching capabilities that
 *   are supported by the device.
 */
static int
cds_get_search_capabilities (dlna_t *dlna, upnp_action_event_t *ev)
{
  if (!dlna || !ev)
  {
    ev->ar->ErrCode = CDS_ERR_ACTION_FAILED;
    return 0;
  }

  upnp_add_response (ev, CDS_ARG_SEARCH_CAPS, "");
  
  return ev->status;
}

/*
 * GetSortCapabilities:
 *   Returns the CSV list of meta-data tags that can be used in sortCriteria.
 */
static int
cds_get_sort_capabilities (dlna_t *dlna, upnp_action_event_t *ev)
{
  if (!dlna || !ev)
  {
    ev->ar->ErrCode = CDS_ERR_ACTION_FAILED;
    return 0;
  }

  upnp_add_response (ev, CDS_ARG_SORT_CAPS, "");
  
  return ev->status;
}

/*
 * GetSystemUpdateID:
 *   This action returns the current value of state variable SystemUpdateID.
 *   It can be used by clients that want to poll for any changes in
 *   the Content Directory (as opposed to subscribing to events).
 */
static int
cds_get_system_update_id (dlna_t *dlna, upnp_action_event_t *ev)
{
  char *SystemUpdateID;
  if (!dlna || !ev)
  {
    ev->ar->ErrCode = CDS_ERR_ACTION_FAILED;
    return 0;
  }

  SystemUpdateID = calloc (1, 256);
  snprintf (SystemUpdateID, 255, "%u", dlna->vfs_root->u.container.updateID);
  upnp_add_response (ev, CDS_ARG_UPDATE_ID,
                     SystemUpdateID);
  free (SystemUpdateID);

  return ev->status;
}

static int
cds_browse_metadata (dlna_t *dlna, upnp_action_event_t *ev,
                     buffer_t *out, vfs_item_t *item, char *filter)
{
  int result_count = 0;
  char *updateID;

  if (!item)
    return -1;

  updateID = calloc (1, 256);
  didl_add_header (out);
  switch (item->type)
  {
  case DLNA_RESOURCE:
    didl_add_item (dlna, out, item, "false", filter);
    snprintf (updateID, 255, "%u", dlna->vfs_root->u.container.updateID);
    break;

  case DLNA_CONTAINER:
    didl_add_container (out, item, "true", "true", CDS_OBJECT_CONTAINER);
    snprintf (updateID, 255, "%u", item->u.container.updateID);
    result_count = 1;
    break;

  default:
    break;
  }
  didl_add_footer (out);
  
  upnp_add_response (ev, CDS_DIDL_RESULT, out->buf);
  upnp_add_response (ev, CDS_DIDL_NUM_RETURNED, "1");
  upnp_add_response (ev, CDS_DIDL_TOTAL_MATCH, "1");
  upnp_add_response (ev, CDS_DIDL_UPDATE_ID,
                     updateID);
  free (updateID);

  return result_count;
}

static int
cds_browse_directchildren (dlna_t *dlna, upnp_action_event_t *ev,
                           buffer_t *out, int index,
                           int count, vfs_item_t *item, char *filter)
{
  vfs_item_t **items;
  int s, result_count = 0;
  char tmp[32];
  char *updateID;

  /* browsing direct children only has a sense on containers */
  if (item->type != DLNA_CONTAINER)
    return -1;
  
  didl_add_header (out);

  /* go to the child pointed out by index */
  items = item->u.container.children;
  for (s = 0; s < index; s++)
    if (*items)
      items++;

  /* UPnP CDS compliance : If starting index = 0 and requested count = 0
     then all children must be returned */
  if (index == 0 && count == 0)
    count = item->u.container.children_count;

  for (; *items; items++)
  {
    /* only fetch the requested count number or all entries if count = 0 */
    if (count == 0 || result_count < count)
    {
      switch ((*items)->type)
      {
      case DLNA_CONTAINER:
        didl_add_container (out, *items, "true", NULL, CDS_OBJECT_CONTAINER);
        break;

      case DLNA_RESOURCE:
        didl_add_item (dlna, out, *items, "true", filter);
        break;

      default:
        break;
      }
      result_count++;
    }
  }

  didl_add_footer (out);

  upnp_add_response (ev, CDS_DIDL_RESULT, out->buf);
  sprintf (tmp, "%d", result_count);
  upnp_add_response (ev, CDS_DIDL_NUM_RETURNED, tmp);
  sprintf (tmp, "%d", item->u.container.children_count);
  upnp_add_response (ev, CDS_DIDL_TOTAL_MATCH, tmp);

  updateID = calloc (1, 256);
  snprintf (updateID, 255, "%u", item->u.container.updateID);
  upnp_add_response (ev, CDS_DIDL_UPDATE_ID,
                     updateID);
  free (updateID);

  return result_count;
}

/*
 * Browse:
 *   This action allows the caller to incrementally browse the native
 *   hierarchy of the Content Directory objects exposed by the Content
 *   Directory Service, including information listing the classes of objects
 *   available in any particular object container.
 */
static int
cds_browse (dlna_t *dlna, upnp_action_event_t *ev)
{
  /* input arguments */
  uint32_t id, index, count, sort;
  char *flag = NULL, *filter = NULL;

  /* output arguments */
  buffer_t *out = NULL;
  int result_count = 0;
  vfs_item_t *item;
  int meta;
  
  if (!dlna || !ev)
  {
    ev->ar->ErrCode = CDS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Check for status */
  if (!ev->status)
  {
    ev->ar->ErrCode = CDS_ERR_ACTION_FAILED;
    return 0;
  }
  
  /* Retrieve input arguments */
  id     = upnp_get_ui4    (ev->ar, CDS_ARG_OBJECT_ID);
  flag   = upnp_get_string (ev->ar, CDS_ARG_BROWSE_FLAG);
  filter = upnp_get_string (ev->ar, CDS_ARG_FILTER);
  index  = upnp_get_ui4    (ev->ar, CDS_ARG_START_INDEX);
  count  = upnp_get_ui4    (ev->ar, CDS_ARG_REQUEST_COUNT);
  sort   = upnp_get_ui4    (ev->ar, CDS_ARG_SORT_CRIT);

  if (!flag || !filter)
  {
    ev->ar->ErrCode = CDS_ERR_INVALID_ARGS;
    goto browse_err;
  }
 
  /* check for arguments validity */
  if (!strcmp (flag, CDS_BROWSE_METADATA))
  {
    if (index)
    {
      ev->ar->ErrCode = CDS_ERR_PROCESS_REQUEST;
      goto browse_err;
    }
    meta = 1;
    }
  else if (!strcmp (flag, CDS_BROWSE_CHILDREN))
    meta = 0;
  else
  {
    ev->ar->ErrCode = CDS_ERR_PROCESS_REQUEST;
    goto browse_err;
  }
  free (flag);

  /* find requested item in VFS */
  item = vfs_get_item_by_id (dlna, id);
  if (!item)
    item = vfs_get_item_by_id (dlna, 0);

  if (!item)
  {
    ev->ar->ErrCode = CDS_ERR_INVALID_OBJECT_ID;
    goto browse_err;
  }

  out = buffer_new ();
  result_count = meta ?
    cds_browse_metadata (dlna, ev, out, item, filter) :
    cds_browse_directchildren (dlna, ev, out, index, count, item, filter);
  
  free (filter);

  if (result_count < 0)
  {
    ev->ar->ErrCode = CDS_ERR_ACTION_FAILED;
    goto browse_err;
  }

  buffer_free (out);
  return ev->status;

 browse_err:
  if (flag)
    free (flag);
  if (filter)
    free (filter);
  if (out)
    buffer_free (out);

  return 0;
}

static int
cds_search_match (dlna_t *dlna, vfs_item_t *item, char *search_criteria)
{
  char keyword[256];
  int derived_from = 0, protocol_contains = 0, result = 0;
  char *and_clause = NULL;
  char *protocol_info;
  char *object_type = NULL;
  dlna_item_t *dlna_item;
  
  if (!item || !search_criteria)
    return 0;

  memset (keyword, '\0', sizeof (keyword));
  
  if (!strncmp (search_criteria, SEARCH_CLASS_MATCH_KEYWORD,
                strlen (SEARCH_CLASS_MATCH_KEYWORD)))
  {
    strncpy (keyword, search_criteria
             + strlen (SEARCH_CLASS_MATCH_KEYWORD), sizeof (keyword));
  }
  else if (!strncmp (search_criteria, SEARCH_CLASS_DERIVED_KEYWORD,
                     strlen (SEARCH_CLASS_DERIVED_KEYWORD)))
  {
    derived_from = 1;
    strncpy (keyword, search_criteria
             + strlen (SEARCH_CLASS_DERIVED_KEYWORD), sizeof (keyword));
  }
  else if (!strncmp (search_criteria, SEARCH_PROTOCOL_CONTAINS_KEYWORD,
                     strlen (SEARCH_PROTOCOL_CONTAINS_KEYWORD)))
  {
    protocol_contains = 1;
    strncpy (keyword, search_criteria
             + strlen (SEARCH_PROTOCOL_CONTAINS_KEYWORD), sizeof (keyword));
  }
  else
    strcpy (keyword, SEARCH_OBJECT_KEYWORD);

  dlna_item = dlna_item_get(dlna, item);
  protocol_info =
    dlna_write_protocol_info (dlna, DLNA_PROTOCOL_INFO_TYPE_HTTP,
                              DLNA_ORG_PLAY_SPEED_NORMAL,
                              item->u.resource.cnv,
                              DLNA_ORG_OPERATION_RANGE,
                              dlna->flags, dlna_item->profile);

  object_type = dlna_profile_upnp_object_item (dlna_item->profile);
  
  if (derived_from && object_type
      && !strncmp (object_type, keyword, strlen (keyword)))
    result = 1;
  else if (protocol_contains && strstr (protocol_info, keyword))
    result = 1;
  else if (object_type && !strcmp (object_type, keyword))
    result = 1;
  free (protocol_info);
  
  and_clause = strstr (search_criteria, SEARCH_AND);
  if (and_clause)
    return (result && cds_search_match (dlna, item,
                                        and_clause + strlen (SEARCH_AND) -1));

  return 1;
}

static int
cds_search_recursive (dlna_t *dlna, vfs_item_t *item, buffer_t *out,
                      int count, char *filter, char *search_criteria)
{
  vfs_item_t **items;
  int result_count = 0;

  if (item->type != DLNA_CONTAINER)
    return 0;
  
  /* go to the first child */
  items = item->u.container.children;
  
  for (; *items; items++)
  {
    /* only fetch the requested count number or all entries if count = 0 */
    if (count == 0 || result_count < count)
    {
      switch ((*items)->type)
      {
      case DLNA_CONTAINER:
        result_count +=
          cds_search_recursive (dlna, *items, out,
                                (count == 0) ? 0 : (count - result_count),
                                filter, search_criteria);
        break;

      case DLNA_RESOURCE:        
        if (cds_search_match (dlna, *items, search_criteria))
        {
          didl_add_item (dlna, out, *items, "true", filter);
          result_count++;
        }
        break;

      default:
        break;
      }
    }
  }

  return result_count;
}

static int
cds_search_directchildren (dlna_t *dlna, upnp_action_event_t *ev,
                           vfs_item_t *item, buffer_t *out, int index,
                           int count, char *filter, char *search_criteria)
{
  vfs_item_t **items;
  int i, result_count = 0;
  char tmp[32];
  
  index = 0;

  /* searching only has a sense on containers */
  if (item->type != DLNA_CONTAINER)
    return -1;
  
  didl_add_header (out);

  /* go to the child pointed out by index */
  items = item->u.container.children;
  for (i = 0; i < index; i++)
    if (*items)
      items++;

  /* UPnP CDS compliance : If starting index = 0 and requested count = 0
     then all children must be returned */
  if (index == 0 && count == 0)
    count = item->u.container.children_count;

  result_count =
    cds_search_recursive (dlna, item, out, count, filter, search_criteria);

  didl_add_footer (out);

  upnp_add_response (ev, CDS_DIDL_RESULT, out->buf);
  sprintf (tmp, "%d", result_count);
  upnp_add_response (ev, CDS_DIDL_NUM_RETURNED, tmp);
  sprintf (tmp, "%d", result_count);
  upnp_add_response (ev, CDS_DIDL_TOTAL_MATCH, tmp);

  return result_count;
}

/*
 * Search:
 *   This action allows the caller to search the content directory for
 *   objects that match some search criteria.
 *   The search criteria are specified as a query string operating on
 *   properties with comparison and logical operators.
 */
static int
cds_search (dlna_t *dlna, upnp_action_event_t *ev)
{
  /* input arguments */
  uint32_t index, count, id, sort_criteria;
  char *search_criteria = NULL, *filter = NULL;

  /* output arguments */
  buffer_t *out = NULL;
  vfs_item_t *item;
  int result_count = 0;
  
  if (!dlna || !ev)
  {
    ev->ar->ErrCode = CDS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Check for status */
  if (!ev->status)
  {
    ev->ar->ErrCode = CDS_ERR_ACTION_FAILED;
    return 0;
  }

  /* Retrieve input arguments */
  id              = upnp_get_ui4    (ev->ar, CDS_ARG_OBJECT_ID);
  search_criteria = upnp_get_string (ev->ar,
                                     CDS_ARG_SEARCH_CRIT);
  filter          = upnp_get_string (ev->ar, CDS_ARG_FILTER);
  index           = upnp_get_ui4    (ev->ar, CDS_ARG_START_INDEX);
  count           = upnp_get_ui4    (ev->ar, CDS_ARG_REQUEST_COUNT);
  sort_criteria   = upnp_get_ui4    (ev->ar, CDS_ARG_SORT_CRIT);

  if (!search_criteria || !filter)
  {
    ev->ar->ErrCode = CDS_ERR_INVALID_ARGS;
    goto search_err;
  }

  /* find requested item in VFS */
  item = vfs_get_item_by_id (dlna, id);
  if (!item)
    item = vfs_get_item_by_id (dlna, 0);

  if (!item)
  {
    ev->ar->ErrCode = CDS_ERR_INVALID_CONTAINER;
    goto search_err;
  }
  
  out = buffer_new ();
  result_count = cds_search_directchildren (dlna, ev, item, out, index, count,
                                            filter, search_criteria);

  if (result_count < 0)
  {
    ev->ar->ErrCode = CDS_ERR_ACTION_FAILED;
    goto search_err;
  }
  
  buffer_free (out);
  upnp_add_response (ev, CDS_DIDL_UPDATE_ID,
                     CDS_ROOT_OBJECT_ID);

  free (search_criteria);
  free (filter);

  return ev->status;

 search_err:
  if (search_criteria)
    free (search_criteria);
  if (filter)
    free (filter);
  if (out)
    buffer_free (out);

  return 0;
}

/* List of UPnP ContentDirectory Service actions */
upnp_service_action_t cds_service_actions[] = {
  /* CDS Required Actions */
  { CDS_ACTION_SEARCH_CAPS, 
    ACTION_ARG_OUT(CDS_ARG_SEARCH_CAPS,"SearchCapabilities") ,
    cds_get_search_capabilities },
  { CDS_ACTION_SORT_CAPS,
    ACTION_ARG_OUT(CDS_ARG_SORT_CAPS,"SortCapabilities") ,
    cds_get_sort_capabilities },
  { CDS_ACTION_UPDATE_ID,
    ACTION_ARG_OUT(CDS_ARG_ID,"SystemUpdateID"),
    cds_get_system_update_id },
  { CDS_ACTION_BROWSE,
    ACTION_ARG_IN(CDS_ARG_OBJECT_ID,"A_ARG_TYPE_ObjectID") \
    ACTION_ARG_IN(CDS_ARG_BROWSE_FLAG,"A_ARG_TYPE_BrowseFlag") \
    ACTION_ARG_IN(CDS_ARG_FILTER,"A_ARG_TYPE_Filter") \
    ACTION_ARG_IN(CDS_ARG_START_INDEX,"A_ARG_TYPE_Index") \
    ACTION_ARG_IN(CDS_ARG_REQUEST_COUNT,"A_ARG_TYPE_Count") \
    ACTION_ARG_IN(CDS_ARG_SORT_CRIT,"A_ARG_TYPE_SortCriteria") \
    ACTION_ARG_OUT(CDS_ARG_RESULT,"A_ARG_TYPE_Result") \
    ACTION_ARG_OUT(CDS_ARG_NUM_RETURNED,"A_ARG_TYPE_Count") \
    ACTION_ARG_OUT(CDS_ARG_TOTAL_MATCHES,"A_ARG_TYPE_Count") \
    ACTION_ARG_OUT(CDS_ARG_UPDATE_ID,"A_ARG_TYPE_UpdateID"),
    cds_browse },

  /* CDS Optional Actions */
  { CDS_ACTION_SEARCH,
    ACTION_ARG_IN(CDS_ARG_CONTAINER_ID,"A_ARG_TYPE_ObjectID") \
    ACTION_ARG_IN(CDS_ARG_SEARCH_CRIT,"A_ARG_TYPE_SearchCriteria") \
    ACTION_ARG_IN(CDS_ARG_FILTER,"A_ARG_TYPE_Filter") \
    ACTION_ARG_IN(CDS_ARG_START_INDEX,"A_ARG_TYPE_Index") \
    ACTION_ARG_IN(CDS_ARG_REQUEST_COUNT,"A_ARG_TYPE_Count") \
    ACTION_ARG_IN(CDS_ARG_SORT_CRIT,"A_ARG_TYPE_SortCriteria") \
    ACTION_ARG_OUT(CDS_ARG_RESULT,"A_ARG_TYPE_Result") \
    ACTION_ARG_OUT(CDS_ARG_NUM_RETURNED,"A_ARG_TYPE_Count") \
    ACTION_ARG_OUT(CDS_ARG_TOTAL_MATCHES,"A_ARG_TYPE_Count") \
    ACTION_ARG_OUT(CDS_ARG_UPDATE_ID,"A_ARG_TYPE_UpdateID") ,
    cds_search },
  { CDS_ACTION_CREATE_OBJ, NULL,     NULL },
  { CDS_ACTION_DESTROY_OBJ, NULL,    NULL },
  { CDS_ACTION_UPDATE_OBJ, NULL,     NULL },
  { CDS_ACTION_IMPORT_RES, NULL,     NULL },
  { CDS_ACTION_EXPORT_RES, NULL,     NULL },
  { CDS_ACTION_STOP_TRANSFER, NULL,  NULL },
  { CDS_ACTION_GET_PROGRESS, NULL,   NULL },
  { CDS_ACTION_DELETE_RES, NULL,     NULL },
  { CDS_ACTION_CREATE_REF, NULL,     NULL },

  /* CDS Vendor-specific Actions */ 
  { NULL, NULL,                              NULL }
};

upnp_service_statevar_t cds_service_variables[] = {
  { "SearchCapabilities", E_STRING, 0, NULL},
  { "SortCapabilities", E_STRING, 0, NULL},
  { "SystemUpdateID", E_UI4, 1, NULL},
  { "ContainerUpdateIDs", E_UI4, 1, NULL},
  { "ServiceResetToken", E_STRING, 0, NULL},
  { "LastChange", E_STRING, 1, NULL},
  { "TransferIDs", E_STRING, 1, NULL},
  { "FeatureList", E_STRING, 0, NULL},
  { "DeviceMode", E_STRING, 1, NULL},
  { "A_ARG_TYPE_ObjectID", E_STRING, 0, NULL},
  { "A_ARG_TYPE_Result", E_STRING, 0, NULL},
  { "A_ARG_TYPE_SearchCriteria", E_STRING, 0, NULL},
  { "A_ARG_TYPE_BrowseFlag", E_STRING, 0, NULL},
  { "A_ARG_TYPE_Filter", E_STRING, 0, NULL},
  { "A_ARG_TYPE_SortCriteria", E_STRING, 0, NULL},
  { "A_ARG_TYPE_Index", E_UI4, 0, NULL},
  { "A_ARG_TYPE_Count", E_UI4, 0, NULL},
  { "A_ARG_TYPE_UpdateID", E_UI4, 0, NULL},
  { "A_ARG_TYPE_TransferID", E_UI4, 0, NULL},
  { "A_ARG_TYPE_TransferStatus", E_STRING, 0, NULL},
  { "A_ARG_TYPE_TransferLength", E_STRING, 0, NULL},
  { "A_ARG_TYPE_TransferTotal", E_STRING, 0, NULL},
  { "A_ARG_TYPE_TagValueList", E_STRING, 0, NULL},
  { "A_ARG_TYPE_URI", E_URI, 0, NULL},
  { NULL, 0, 0, NULL},
};

static char *
cds_get_description (dlna_t *dlna)
{
  return dlna_service_get_description (dlna, cds_service_actions, cds_service_variables);
}

dlna_service_t cds_service = {
  .id           = CDS_SERVICE_ID,
  .type         = CDS_SERVICE_TYPE,
  .scpd_url     = CDS_URL,
  .control_url  = CDS_CONTROL_URL,
  .event_url    = CDS_EVENT_URL,
  .actions      = cds_service_actions,
  .statevar      = cds_service_variables,
  .get_description     = cds_get_description,
  .init         = NULL,
};
