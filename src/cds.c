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
#include "cms.h"
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
#define CDS_ACTION_GET_FEATURE_LIST   "GetFeatureList"

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
#define CDS_ARG_ID                    "Id"

#define CDS_ARG_CONTAINER_ID          "ContainerID"
#define CDS_ARG_FEATURE_LIST          "FeatureList"

/* CDS Argument Values */
#define CDS_ROOT_OBJECT_ID            "0"
#define CDS_BROWSE_METADATA           "BrowseMetadata"
#define CDS_BROWSE_CHILDREN           "BrowseDirectChildren"
#define CDS_OBJECT_CONTAINER          "object.container.storageFolder"
#define CDS_DIDL_RESULT               "Result"
#define CDS_DIDL_NUM_RETURNED         "NumberReturned"
#define CDS_DIDL_TOTAL_MATCH          "TotalMatches"
#define CDS_DIDL_UPDATE_ID            "UpdateID"

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

enum
{
SearchCapabilities,
SortCapabilities,
SystemUpdateID,
ContainerUpdateIDs,
ServiceResetToken,
LastChange,
TransferIDs,
FeatureList,
DeviceMode,
A_ARG_TYPE_ObjectID,
A_ARG_TYPE_Result,
A_ARG_TYPE_SearchCriteria,
A_ARG_TYPE_BrowseFlag,
A_ARG_TYPE_Filter,
A_ARG_TYPE_SortCriteria,
A_ARG_TYPE_Index,
A_ARG_TYPE_Count,
A_ARG_TYPE_UpdateID,
A_ARG_TYPE_TransferID,
A_ARG_TYPE_TransferStatus,
A_ARG_TYPE_TransferLength,
A_ARG_TYPE_TransferTotal,
A_ARG_TYPE_TagValueList,
A_ARG_TYPE_URI,
};
upnp_service_statevar_t cds_service_variables[];

typedef struct cds_feature_s cds_feature_t;

struct cds_feature_s
{
  char *name;
  cds_feature_t *next;
};

struct cds_data_s
{
  dlna_vfs_t *vfs;
  cds_feature_t *features;
  char *sort_caps;
  char *search_caps;
};
typedef struct cds_data_s cds_data_t;
/*
 * GetSearchCapabilities:
 *   This action returns the searching capabilities that
 *   are supported by the device.
 */
static char *
cds_search_capabilities (dlna_t *dlna dlna_unused, dlna_service_t *service)
{
  cds_data_t *cds_data;

  if (!service)
    return NULL;
  cds_data = (cds_data_t *)service->cookie;
  return cds_data->search_caps;
}

static int
cds_get_search_capabilities (dlna_t *dlna, upnp_action_event_t *ev)
{
  char *value = NULL;

  if (!dlna || !ev)
  {
    ev->ar->ErrCode = CDS_ERR_ACTION_FAILED;
    return 0;
  }

  if (cds_service_variables[SearchCapabilities].get)
    value = cds_service_variables[SearchCapabilities].get (NULL, ev->service);
  if (value)
  {
    upnp_add_response (ev, CDS_ARG_SEARCH_CAPS, value);
    free (value);
  }
  else
    upnp_add_response (ev, CDS_ARG_SEARCH_CAPS, "");
  
  return ev->status;
}

/*
 * GetSortCapabilities:
 *   Returns the CSV list of meta-data tags that can be used in sortCriteria.
 */
static char *
cds_sort_capabilities (dlna_t *dlna dlna_unused, dlna_service_t *service)
{
  cds_data_t *cds_data;

  if (!service)
    return NULL;
  return "dc:title";
}

static int
cds_get_sort_capabilities (dlna_t *dlna, upnp_action_event_t *ev)
{
  char *value = NULL;

  if (!dlna || !ev)
  {
    ev->ar->ErrCode = CDS_ERR_ACTION_FAILED;
    return 0;
  }

  if (cds_service_variables[SortCapabilities].get)
    value = cds_service_variables[SortCapabilities].get (NULL, ev->service);
  if (value)
  {
    upnp_add_response (ev, CDS_ARG_SORT_CAPS, value);
    free (value);
  }
  else
    upnp_add_response (ev, CDS_ARG_SORT_CAPS, "");

  return ev->status;
}

/*
 * GetSystemUpdateID:
 *   This action returns the current value of state variable SystemUpdateID.
 *   It can be used by clients that want to poll for any changes in
 *   the Content Directory (as opposed to subscribing to events).
 */
static char *
cds_system_update_id (dlna_t *dlna dlna_unused, dlna_service_t *service)
{
  char *value;
  cds_data_t *cds_data;
  dlna_vfs_t *vfs;

  if (!service)
    return NULL;
  cds_data = (cds_data_t *)service->cookie;
  vfs = (dlna_vfs_t *) cds_data->vfs;
  value = calloc (1, 11);
  snprintf (value, 10, "%10u", vfs->vfs_root->u.container.updateID);
  return value;
}

static int
cds_get_system_update_id (dlna_t *dlna, upnp_action_event_t *ev)
{
  if (!dlna || !ev)
  {
    ev->ar->ErrCode = CDS_ERR_ACTION_FAILED;
    return 0;
  }
  if (cds_service_variables[SystemUpdateID].get)
  {
    char *value;
    value = cds_service_variables[SystemUpdateID].get (NULL, ev->service);
    upnp_add_response (ev, CDS_ARG_ID, value);
    free (value);
  }
  else
    upnp_add_response (ev, CDS_ARG_ID,CDS_ROOT_OBJECT_ID);

  return ev->status;
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
  cds_data_t *cds_data = (cds_data_t *)ev->service->cookie;
  dlna_vfs_t *vfs = cds_data->vfs;
  /* input arguments */
  uint32_t id, index, count;
  char *flag = NULL, *filter = NULL, *sort = NULL;

  /* output arguments */
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
  sort   = upnp_get_string (ev->ar, CDS_ARG_SORT_CRIT);

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

  didl_result_t result;
  result.out = buffer_new ();

  /* find requested item in VFS */
  item = vfs_get_item_by_id (vfs, id);

  if (!item)
  {
    ev->ar->ErrCode = CDS_ERR_INVALID_OBJECT_ID;
    goto browse_err;
  }

  result_count = meta ?
    vfs_browse_metadata (item, filter, &result) :
    vfs_browse_directchildren (item, index, count, filter, sort, &result);

  if (result_count < 0)
  {
    ev->ar->ErrCode = CDS_ERR_ACTION_FAILED;
    goto browse_err;
  }

  char tmp[11];

  upnp_add_response (ev, CDS_DIDL_RESULT, result.out->buf);
  sprintf (tmp, "%10lu", result.nb_returned);
  upnp_add_response (ev, CDS_DIDL_NUM_RETURNED, tmp);
  sprintf (tmp, "%10lu", result.total_match);
  upnp_add_response (ev, CDS_DIDL_TOTAL_MATCH, tmp);
  sprintf (tmp, "%10lu", result.updateid);
  upnp_add_response (ev, CDS_DIDL_UPDATE_ID, tmp);
  buffer_free (result.out);


  free (flag);
  free (filter);
  free (sort);

  return ev->status;

 browse_err:
  if (flag)
    free (flag);
  if (filter)
    free (filter);
  if (sort)
    free (sort);

  return 0;
}

static int
cds_search_directchildren (dlna_t *dlna dlna_unused, upnp_action_event_t *ev,
                           vfs_item_t *item, int index,
                           int count, char *filter, char *search_criteria)
{
  int result_count = 0;
  char tmp[11];

  index = 0;

  /* searching only has a sense on containers */
  if (item->type != DLNA_CONTAINER)
    return -1;

  didl_result_t result;
  result.out = buffer_new ();

  result_count = vfs_search_directchildren (item, 
                  index, count, filter, search_criteria, &result);
  upnp_add_response (ev, CDS_DIDL_RESULT, result.out->buf);
  sprintf (tmp, "%10lu", result.nb_returned);
  upnp_add_response (ev, CDS_DIDL_NUM_RETURNED, tmp);
  sprintf (tmp, "%10lu", result.total_match);
  upnp_add_response (ev, CDS_DIDL_TOTAL_MATCH, tmp);
  buffer_free (result.out);

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
  cds_data_t *cds_data = (cds_data_t *)ev->service->cookie;
  dlna_vfs_t *vfs = cds_data->vfs;
  /* input arguments */
  uint32_t index, count, id;
  char *search_criteria = NULL, *filter = NULL, *sort = NULL;

  /* output arguments */
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
  sort            = upnp_get_string (ev->ar, CDS_ARG_SORT_CRIT);

  if (!search_criteria || !filter)
  {
    ev->ar->ErrCode = CDS_ERR_INVALID_ARGS;
    goto search_err;
  }

  /* find requested item in VFS */
  item = vfs_get_item_by_id (vfs, id);
  if (!item)
    item = vfs_get_item_by_id (vfs, 0);

  if (!item)
  {
    ev->ar->ErrCode = CDS_ERR_INVALID_CONTAINER;
    goto search_err;
  }
  
  result_count = cds_search_directchildren (dlna, ev, item, index, count,
                                            filter, search_criteria);

  if (result_count < 0)
  {
    ev->ar->ErrCode = CDS_ERR_ACTION_FAILED;
    goto search_err;
  }
  
  upnp_add_response (ev, CDS_DIDL_UPDATE_ID,
                     CDS_ROOT_OBJECT_ID);

  free (search_criteria);
  free (filter);
  free (sort);

  return ev->status;

 search_err:
  if (search_criteria)
    free (search_criteria);
  if (filter)
    free (filter);
  if (sort)
    free (sort);

  return 0;
}

/*
 * GetFeatureList:
 */
static int
cds_get_feature_list (dlna_t *dlna dlna_unused, upnp_action_event_t *ev)
{
  if (!dlna || !ev)
  {
    ev->ar->ErrCode = CDS_ERR_ACTION_FAILED;
    return 0;
  }

  if (cds_service_variables[FeatureList].get)
  {
    char *value;
    value = cds_service_variables[FeatureList].get (NULL, ev->service);
    upnp_add_response (ev, CDS_ARG_FEATURE_LIST, value);
    free (value);
  }
  else
    upnp_add_response (ev, CDS_ARG_FEATURE_LIST, "");
  return ev->status;
}

static char *
cds_feature_list (dlna_t *dlna dlna_unused, dlna_service_t *service)
{
  cds_data_t *cds_data = (cds_data_t *)service->cookie;
  cds_feature_t *feature;
  buffer_t *out;
  IXML_Document *featurelistDoc;
  IXML_Node *first;

  out = buffer_new ();
  buffer_append (out, "<Features xmlns=\"urn:schemas-upnp-org:av:avs\" ");
  buffer_append (out, "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"");
  buffer_append (out, "xsi:schemaLocation=\"urn:schemas-upnp-org:av:avs http://www.upnp.org/schemas/av/avs.xsd\">");
  buffer_append (out, "</Features>");

  ixmlParseBufferEx( out->buf,&featurelistDoc);
  buffer_free (out);
  
  first = ixmlNode_getFirstChild( ( IXML_Node * ) featurelistDoc );
  for (feature = cds_data->features; feature; feature = feature->next)
  {
    IXML_Element *elem;

    elem = ixmlDocument_createElement( featurelistDoc, "Feature" );
    ixmlElement_setAttribute (elem, "name", feature->name);
    ixmlElement_setAttribute (elem, "version", "1");
    ixmlNode_appendChild (first, (IXML_Node *)elem);
  }
  return ixmlPrintDocument (featurelistDoc);
}


upnp_service_action_arg_t *browse_args[] =
{
  &(upnp_service_action_arg_t){.name = CDS_ARG_OBJECT_ID,
   .dir = E_INPUT,
   .relation = &cds_service_variables[A_ARG_TYPE_ObjectID]},
  &(upnp_service_action_arg_t){.name = CDS_ARG_BROWSE_FLAG,
   .dir = E_INPUT,
   .relation = &cds_service_variables[A_ARG_TYPE_BrowseFlag]},
  &(upnp_service_action_arg_t){.name = CDS_ARG_FILTER,
   .dir = E_INPUT,
   .relation = &cds_service_variables[A_ARG_TYPE_Filter]},
  &(upnp_service_action_arg_t){.name = CDS_ARG_START_INDEX,
   .dir = E_INPUT,
   .relation = &cds_service_variables[A_ARG_TYPE_Index]},
  &(upnp_service_action_arg_t){.name = CDS_ARG_REQUEST_COUNT,
   .dir = E_INPUT,
   .relation = &cds_service_variables[A_ARG_TYPE_Count]},
  &(upnp_service_action_arg_t){.name = CDS_ARG_SORT_CRIT,
   .dir = E_INPUT,
   .relation = &cds_service_variables[A_ARG_TYPE_SortCriteria]},
  &(upnp_service_action_arg_t){.name = CDS_ARG_RESULT,
   .dir = E_OUTPUT,
   .relation = &cds_service_variables[A_ARG_TYPE_Result]},
  &(upnp_service_action_arg_t){.name = CDS_ARG_NUM_RETURNED,
   .dir = E_OUTPUT,
   .relation = &cds_service_variables[A_ARG_TYPE_Count]},
  &(upnp_service_action_arg_t){.name = CDS_ARG_TOTAL_MATCHES,
   .dir = E_OUTPUT,
   .relation = &cds_service_variables[A_ARG_TYPE_Count]},
  &(upnp_service_action_arg_t){.name = CDS_ARG_UPDATE_ID,
   .dir = E_OUTPUT,
   .relation = &cds_service_variables[A_ARG_TYPE_UpdateID]},
  NULL
};

upnp_service_action_arg_t *getfeaturelist_args[] = {
  &(upnp_service_action_arg_t){.name=CDS_ARG_FEATURE_LIST, .dir=E_OUTPUT, .relation=&cds_service_variables[FeatureList]},
  NULL
};

/* List of UPnP ContentDirectory Service actions */
upnp_service_action_t cds_service_actions[] = {
  /* CDS Required Actions */
  { .name = CDS_ACTION_SEARCH_CAPS, 
    .args = ACTION_ARG_OUT(CDS_ARG_SEARCH_CAPS,"SearchCapabilities") ,
    .args_s = NULL,
    .cb = cds_get_search_capabilities },
  { .name = CDS_ACTION_SORT_CAPS,
    .args = ACTION_ARG_OUT(CDS_ARG_SORT_CAPS,"SortCapabilities") ,
    .args_s = NULL,
    .cb = cds_get_sort_capabilities },
  { .name = CDS_ACTION_UPDATE_ID,
    .args = ACTION_ARG_OUT(CDS_ARG_ID,"SystemUpdateID"),
    .args_s = NULL,
    .cb = cds_get_system_update_id },
  { .name = CDS_ACTION_BROWSE,
    .args = NULL,
    .args_s = browse_args,
    .cb = cds_browse },

  /* CDS Optional Actions */
  { .name = CDS_ACTION_SEARCH,
    .args = ACTION_ARG_IN(CDS_ARG_CONTAINER_ID,"A_ARG_TYPE_ObjectID") \
    ACTION_ARG_IN(CDS_ARG_SEARCH_CRIT,"A_ARG_TYPE_SearchCriteria") \
    ACTION_ARG_IN(CDS_ARG_FILTER,"A_ARG_TYPE_Filter") \
    ACTION_ARG_IN(CDS_ARG_START_INDEX,"A_ARG_TYPE_Index") \
    ACTION_ARG_IN(CDS_ARG_REQUEST_COUNT,"A_ARG_TYPE_Count") \
    ACTION_ARG_IN(CDS_ARG_SORT_CRIT,"A_ARG_TYPE_SortCriteria") \
    ACTION_ARG_OUT(CDS_ARG_RESULT,"A_ARG_TYPE_Result") \
    ACTION_ARG_OUT(CDS_ARG_NUM_RETURNED,"A_ARG_TYPE_Count") \
    ACTION_ARG_OUT(CDS_ARG_TOTAL_MATCHES,"A_ARG_TYPE_Count") \
    ACTION_ARG_OUT(CDS_ARG_UPDATE_ID,"A_ARG_TYPE_UpdateID") ,
    .args_s = NULL,
    .cb = cds_search },
  { .name = CDS_ACTION_GET_FEATURE_LIST,
    .args = NULL,
    .args_s = getfeaturelist_args,
    .cb = cds_get_feature_list },
  { .name = CDS_ACTION_CREATE_OBJ,
    .args = NULL,
    .args_s = NULL,
    .cb = NULL },
  { .name = CDS_ACTION_DESTROY_OBJ,
    .args = NULL,
    .args_s = NULL,
    .cb = NULL },
  { .name = CDS_ACTION_UPDATE_OBJ,
    .args = NULL,
    .args_s = NULL,
    .cb = NULL },
  { .name = CDS_ACTION_IMPORT_RES,
    .args = NULL,
    .args_s = NULL,
    .cb = NULL },
  { .name = CDS_ACTION_EXPORT_RES,
    .args = NULL,
    .args_s = NULL,
    .cb = NULL },
  { .name = CDS_ACTION_STOP_TRANSFER,
    .args = NULL,
    .args_s = NULL,
    .cb = NULL },
  { .name = CDS_ACTION_GET_PROGRESS,
    .args = NULL,
    .args_s = NULL,
    .cb = NULL },
  { .name = CDS_ACTION_DELETE_RES,
    .args = NULL,
    .args_s = NULL,
    .cb = NULL },
  { .name = CDS_ACTION_CREATE_REF,
    .args = NULL,
    .args_s = NULL,
    .cb = NULL },

  /* CDS Vendor-specific Actions */ 
  { NULL, NULL, NULL, NULL }
};

char *A_ARG_TYPE_BrowseFlag_allowed[] =
{"BrowseMetadata","BrowseDirectChildren",NULL};
char *A_ARG_TYPE_TransferStatus_allowed[] =
{"COMPLETED","ERROR","IN_PROGRESS","STOPPED",NULL};

upnp_service_statevar_t cds_service_variables[] = {
  [SearchCapabilities] = { "SearchCapabilities", E_STRING, 0, NULL, cds_search_capabilities},
  [SortCapabilities] = { "SortCapabilities", E_STRING, 0, NULL, cds_sort_capabilities},
  [SystemUpdateID] = { "SystemUpdateID", E_UI4, 1, NULL, cds_system_update_id},
  [ContainerUpdateIDs] = { "ContainerUpdateIDs", E_UI4, 1, NULL, NULL},
  [ServiceResetToken] = { "ServiceResetToken", E_STRING, 0, NULL, NULL},
  [LastChange] = { "LastChange", E_STRING, 1, NULL, NULL},
  [TransferIDs] = { "TransferIDs", E_STRING, 1, NULL, NULL},
  [FeatureList] = { "FeatureList", E_STRING, 0, NULL, cds_feature_list},
  [DeviceMode] = { "DeviceMode", E_STRING, 1, NULL, NULL},
  [A_ARG_TYPE_ObjectID] = { "A_ARG_TYPE_ObjectID", E_STRING, 0, NULL, NULL},
  [A_ARG_TYPE_Result] = { "A_ARG_TYPE_Result", E_STRING, 0, NULL, NULL},
  [A_ARG_TYPE_SearchCriteria] = { "A_ARG_TYPE_SearchCriteria", E_STRING, 0, NULL, NULL},
  [A_ARG_TYPE_BrowseFlag] = { "A_ARG_TYPE_BrowseFlag", E_STRING, 0, A_ARG_TYPE_BrowseFlag_allowed, NULL},
  [A_ARG_TYPE_Filter] = { "A_ARG_TYPE_Filter", E_STRING, 0, NULL, NULL},
  [A_ARG_TYPE_SortCriteria] = { "A_ARG_TYPE_SortCriteria", E_STRING, 0, NULL, NULL},
  [A_ARG_TYPE_Index] = { "A_ARG_TYPE_Index", E_UI4, 0, NULL, NULL},
  [A_ARG_TYPE_Count] = { "A_ARG_TYPE_Count", E_UI4, 0, NULL, NULL},
  [A_ARG_TYPE_UpdateID] = { "A_ARG_TYPE_UpdateID", E_UI4, 0, NULL, NULL},
  [A_ARG_TYPE_TransferID] = { "A_ARG_TYPE_TransferID", E_UI4, 0, NULL, NULL},
  [A_ARG_TYPE_TransferStatus] = { "A_ARG_TYPE_TransferStatus", E_STRING, 0, A_ARG_TYPE_TransferStatus_allowed, NULL},
  [A_ARG_TYPE_TransferLength] = { "A_ARG_TYPE_TransferLength", E_STRING, 0, NULL, NULL},
  [A_ARG_TYPE_TransferTotal] = { "A_ARG_TYPE_TransferTotal", E_STRING, 0, NULL, NULL},
  [A_ARG_TYPE_TagValueList] = { "A_ARG_TYPE_TagValueList", E_STRING, 0, NULL, NULL},
  [A_ARG_TYPE_URI] = { "A_ARG_TYPE_URI", E_URI, 0, NULL, NULL},
  { NULL, 0, 0, NULL, NULL},
};

static int
cds_init (dlna_service_t *service)
{
  cds_data_t *cds_data = (cds_data_t *)service->cookie;
  int res;

  res = dlnaAddVirtualDir (VIRTUAL_DIR);
  if (res != DLNA_E_SUCCESS)
  {
    dlna_log (DLNA_MSG_CRITICAL,
              "Cannot add virtual directory for web server\n");
  }
  dlna_service_t *cms = dlna_service_find_id (service->device, DLNA_SERVICE_CONNECTION_MANAGER);
  cms_set_protocol_info (cms, cds_data->vfs->sources, 0);

  return res;
}

static char *
cds_get_description (dlna_service_t *service dlna_unused)
{
  return dlna_service_get_description (cds_service_actions, cds_service_variables);
}

dlna_service_t *
cds_service_new (dlna_t *dlna dlna_unused, dlna_vfs_t *vfs)
{
  dlna_service_t *service = NULL;
  service = calloc (1, sizeof (dlna_service_t));
  
  service->id           = CDS_SERVICE_ID;
  service->type         = CDS_SERVICE_TYPE;
  service->typeid       = DLNA_SERVICE_CONTENT_DIRECTORY;
  service->scpd_url     = CDS_URL;
  service->control_url  = CDS_CONTROL_URL;
  service->event_url    = CDS_EVENT_URL;
  service->actions      = cds_service_actions;
  service->statevar     = cds_service_variables;
  service->get_description     = cds_get_description;
  service->init         = cds_init;
  service->last_change  = 1;

  cds_data_t *cds_data = calloc (1, sizeof (cds_data_t));
  service->cookie = cds_data;
  cds_data->vfs = vfs;

  return service;
};

void
cds_vfs_changed (dlna_service_t *service)
{
  if (service->statevar[SystemUpdateID].eventing)
    service->statevar[SystemUpdateID].eventing++;
  if (!service->statevar[SystemUpdateID].eventing)
  {
    service->statevar[SystemUpdateID].eventing = 1;
  }
}
