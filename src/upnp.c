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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#if (defined(BSD) || defined(__FreeBSD__) || defined(__APPLE__))
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if_dl.h>
#endif

#if (defined(__APPLE__))
#include <net/route.h>
#endif

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "upnp_internals.h"
#include "devices.h"
#include "services.h"

static void
upnp_subscription_request_handler(dlna_t *dlna,
                            struct dlna_Subscription_Request *req)
{
  dlna_service_t *srv;
	int rc;
  IXML_Document *propset = NULL;

  if (!dlna || !req)
    return;

	srv = dlna_service_find (dlna->device, req->ServiceId);
	if (srv && srv->statevar)
  {
    int i;

    for (i = 0; srv->statevar[i].name; i++)
    {
      char *result = NULL;
      char *value = NULL;
      if (srv->statevar[i].eventing && srv->statevar[i].get)
       value = srv->statevar[i].get (dlna, srv);
      if (value)
      {
        result = strdup (value);
        dlnaAddToPropertySet (&propset, srv->statevar[i].name, result);
        free (result);
      }
    }

    rc = dlnaAcceptSubscriptionExt(dlna->dev,
              req->UDN, req->ServiceId, propset, req->Sid);

    ixmlDocument_free (propset);
    if (rc != DLNA_E_SUCCESS)
    {
      dlna_log (dlna, DLNA_MSG_ERROR,
        "Subscription Error: %s (%d)",
          dlnaGetErrorMessage(rc), rc);
    }
  }
	return;
}

static int
upnp_find_service_statevar (dlna_t *dlna,
                          dlna_service_t **service,
                          upnp_service_statevar_t **statevar,
                          struct dlna_State_Var_Request *ar)
{
  int a;
  const dlna_service_t *srv;

  *service = NULL;
  *statevar = NULL;

  if (!ar || !ar->StateVarName)
    return DLNA_ST_ERROR;

  dlna_log (dlna, DLNA_MSG_INFO,
            "StateVariable: using service %s\n", ar->ServiceID);
  
  /* find the resquested service in all registered ones */
  srv = dlna_service_find (dlna->device, ar->ServiceID);
  if (!srv || !srv->statevar)
    return DLNA_ST_ERROR;
  
  /* parse all known actions */
  for (a = 0; srv->statevar[a].name; a++)
  {
    /* find the requested one */
    if (!strcmp (srv->statevar[a].name, ar->StateVarName))
    {
      dlna_log (dlna, DLNA_MSG_INFO,
                "StateVariable: using action %s\n", ar->StateVarName);
      *service = (dlna_service_t *)srv;
      *statevar = &srv->statevar[a];
      return DLNA_ST_OK;
    }
  }

  return DLNA_ST_ERROR;
}

static void
upnp_var_request_handler (dlna_t *dlna, struct dlna_State_Var_Request *ar)
{
  dlna_service_t *service = NULL;
  upnp_service_statevar_t *statevar;

  if (!dlna || !ar)
    return;

  if (ar->ErrCode != DLNA_E_SUCCESS)
    return;

  /* ensure that message target is the specified device */
  if (strcmp (ar->DevUDN + 5, dlna->device->uuid))
    return;
  
  if (dlna->verbosity == DLNA_MSG_INFO)
  {
    char val[256];
    uint32_t ip;

    ip = ((struct in_addr *)&(ar->CtrlPtIPAddr))->s_addr;
    ip = ntohl (ip);
    sprintf (val, "%d.%d.%d.%d",
             (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

    dlna_log (dlna, DLNA_MSG_INFO,
              "***************************************************\n");
    dlna_log (dlna, DLNA_MSG_INFO,
              "**             New State Var Request                **\n");
    dlna_log (dlna, DLNA_MSG_INFO,
              "***************************************************\n");
    dlna_log (dlna, DLNA_MSG_INFO, "ServiceID: %s\n", ar->ServiceID);
    dlna_log (dlna, DLNA_MSG_INFO, "StateVarName: %s\n", ar->StateVarName);
    dlna_log (dlna, DLNA_MSG_INFO, "CtrlPtIP: %s\n", val);
  }

  if (upnp_find_service_statevar (dlna, &service, &statevar, ar) == DLNA_ST_OK)
  {
    char *result = NULL;
    const char *value;

    if (statevar->get)
      value = statevar->get (dlna, service);
    if (value)
      result = strdup (value);
    ar->CurrentVal = result;
    ar->ErrCode = (result == NULL)
      ? DLNA_SOAP_E_INVALID_ACTION
      : DLNA_E_SUCCESS;
  }
}

static int
upnp_find_service_action (dlna_t *dlna,
                          dlna_service_t **service,
                          upnp_service_action_t **action,
                          struct dlna_Action_Request *ar)
{
  int a;
  const dlna_service_t *srv;
  
  *service = NULL;
  *action = NULL;

  if (!ar || !ar->ActionName)
    return DLNA_ST_ERROR;

  dlna_log (dlna, DLNA_MSG_INFO,
            "ActionRequest: using service %s\n", ar->ServiceID);
  
  /* find the resquested service in all registered ones */
  srv = dlna_service_find (dlna->device, ar->ServiceID);
  if (!srv)
    return DLNA_ST_ERROR;
  
  /* parse all known actions */
  for (a = 0; srv->actions[a].name; a++)
  {
    /* find the requested one */
    if (!strcmp (srv->actions[a].name, ar->ActionName))
    {
      dlna_log (dlna, DLNA_MSG_INFO,
                "ActionRequest: using action %s\n", ar->ActionName);
      *service = (dlna_service_t *)srv;
      *action = &srv->actions[a];
      return DLNA_ST_OK;
    }
  }

  return DLNA_ST_ERROR;
}

static void
upnp_action_request_handler (dlna_t *dlna, struct dlna_Action_Request *ar)
{
  dlna_service_t *service = NULL;
  upnp_service_action_t *action;

  if (!dlna || !ar)
    return;

  if (ar->ErrCode != DLNA_E_SUCCESS)
    return;

  /* ensure that message target is the specified device */
  if (strcmp (ar->DevUDN + 5, dlna->device->uuid))
    return;
  
  if (dlna->verbosity == DLNA_MSG_INFO)
  {
    char val[256];
    uint32_t ip;

    ip = ((struct in_addr *)&(ar->CtrlPtIPAddr))->s_addr;
    ip = ntohl (ip);
    sprintf (val, "%d.%d.%d.%d",
             (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

    DOMString str = ixmlPrintDocument (ar->ActionRequest);
    dlna_log (dlna, DLNA_MSG_INFO,
              "***************************************************\n");
    dlna_log (dlna, DLNA_MSG_INFO,
              "**             New Action Request                **\n");
    dlna_log (dlna, DLNA_MSG_INFO,
              "***************************************************\n");
    dlna_log (dlna, DLNA_MSG_INFO, "Device UDN: %s\n", ar->DevUDN);
    dlna_log (dlna, DLNA_MSG_INFO, "ServiceID: %s\n", ar->ServiceID);
    dlna_log (dlna, DLNA_MSG_INFO, "ActionName: %s\n", ar->ActionName);
    dlna_log (dlna, DLNA_MSG_INFO, "CtrlPtIP: %s\n", val);
    dlna_log (dlna, DLNA_MSG_INFO, "Action Request:\n%s\n", str);
    ixmlFreeDOMString (str);
  }

  if (upnp_find_service_action (dlna, &service, &action, ar) == DLNA_ST_OK)
  {
    upnp_action_event_t event;

    event.ar      = ar;
    event.status  = 1;
    event.service = service;
    event.device = dlna->device;

    if (action->cb && action->cb (dlna, &event) && event.status)
    {
      ar->ErrCode = DLNA_E_SUCCESS;
      if (!ar->ActionResult)
      {
        ar->ActionResult = UpnpMakeActionResponse (
                              ar->ActionName, service->type, 0, NULL);
      }
    }

    if (dlna->verbosity == DLNA_MSG_INFO)
    {
      DOMString str = ixmlPrintDocument (ar->ActionResult);
      dlna_log (dlna, DLNA_MSG_INFO, "Action Result:\n%s", str);
      dlna_log (dlna, DLNA_MSG_INFO,
                "***************************************************\n");
      dlna_log (dlna, DLNA_MSG_INFO, "\n");
      ixmlFreeDOMString (str);
    }
      
    return;
  }

  if (service) /* Invalid Action name */
    strcpy (ar->ErrStr, "Unknown Service Action");
  else /* Invalid Service name */
    strcpy (ar->ErrStr, "Unknown Service ID");
  
  ar->ActionResult = NULL;
  ar->ErrCode = DLNA_SOAP_E_INVALID_ACTION;
}

static int
device_callback_event_handler (dlna_EventType type,
                               void *event,
                               void *cookie)
{
  switch (type)
  {
  case DLNA_EVENT_SUBSCRIPTION_REQUEST:
    upnp_subscription_request_handler ((dlna_t *) cookie,
                                 (struct dlna_Subscription_Request *) event);
    break;
  case DLNA_CONTROL_ACTION_REQUEST:
    upnp_action_request_handler ((dlna_t *) cookie,
                                 (struct dlna_Action_Request *) event);
    break;
  case DLNA_CONTROL_GET_VAR_REQUEST:
    upnp_var_request_handler ((dlna_t *) cookie,
                                 (struct dlna_State_Var_Request *) event);
    break;
  case DLNA_CONTROL_ACTION_COMPLETE:
    break;
  default:
    break;
  }

  return 0;
}

static char *
get_iface_address (char *interface)
{
  int sock;
  uint32_t ip;
  struct ifreq ifr;
  char *val;

  if (!interface)
    return NULL;

  /* determine UDN according to MAC address */
  sock = socket (AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror ("socket");
    return NULL;
  }

  strcpy (ifr.ifr_name, interface);
  ifr.ifr_addr.sa_family = AF_INET;

  if (ioctl (sock, SIOCGIFADDR, &ifr) < 0)
  {
    perror ("ioctl");
    close (sock);
    return NULL;
  }

  val = malloc (16 * sizeof (char));
  ip = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr;
  ip = ntohl (ip);
  sprintf (val, "%d.%d.%d.%d",
           (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

  close (sock);
  return val;
}

static int
upnp_service_init (void *cookie, dlna_service_t *service)
{
  if (service->init)
    return service->init (service);
  return 0;
}

static int
upnp_event_notify (void *cookie, dlna_service_t *service)
{
  dlna_t *dlna = (dlna_t *)cookie;
  IXML_Document *propset = NULL;
  int rc = DLNA_E_SUCCESS;

  if (!dlna)
    return -1;

	if (service && service->statevar)
  {
    int i;
    uint32_t last_change = 0;

    for (i = 0; service->statevar[i].name; i++)
    {
      char *value = NULL;
      if (service->statevar[i].eventing && service->statevar[i].get)
       value = service->statevar[i].get (dlna, service);
      if (value && (service->last_change < service->statevar[i].eventing))
      {
        dlnaAddToPropertySet (&propset, service->statevar[i].name, value);
        if (last_change < service->statevar[i].eventing)
          last_change = service->statevar[i].eventing;
      }
      if (value)
        free (value);
    }

    if (service->last_change < last_change)
    {
      service->last_change = last_change;
      for (i = 0; service->statevar[i].name; i++)
      {
        if (service->statevar[i].eventing && (service->last_change > service->statevar[i].eventing))
          service->statevar[i].eventing = service->last_change;
      }
      if (dlna->verbosity == DLNA_MSG_INFO)
      {
        dlna_log (dlna, DLNA_MSG_INFO,
                  "***************************************************\n");
        dlna_log (dlna, DLNA_MSG_INFO,
                  "**             New State Var Notification                **\n");
        dlna_log (dlna, DLNA_MSG_INFO,
                  "***************************************************\n");
        dlna_log (dlna, DLNA_MSG_INFO, "DeviceID: %s\n", dlna->device->uuid);
        dlna_log (dlna, DLNA_MSG_INFO, "ServiceID: %s\n", service->id);
      }
      if (propset)
      {
        buffer_t *udn;
        udn = buffer_new ();
        buffer_appendf (udn, "uuid:%s",dlna->device->uuid);
        rc = dlnaNotifyExt(dlna->dev, udn->buf, service->id, propset);
        buffer_free (udn);
        ixmlDocument_free (propset);
      }
    }

    if (rc != DLNA_E_SUCCESS)
    {
       dlna_log (dlna, DLNA_MSG_ERROR,
        "Event Notify Error: %s (%d)", dlnaGetErrorMessage(rc), rc);
    }
  }
  return rc;
}

static void *
dlna_event_thread (void *arg)
{
  dlna_t *dlna =(dlna_t *)arg;

  while (dlna->inited)
  {
    int ret;
    struct timespec abstime = {.tv_sec = 0, .tv_nsec = 200000000,}; /*0.2s*/
    ithread_mutex_lock (&dlna->event_mutex);
    do
    {
      //ret = ithread_cond_wait (&dlna->eventing, &dlna->event_mutex);
      nanosleep (&abstime, NULL);
      ret = ithread_cond_timedwait (&dlna->eventing, &dlna->event_mutex, &abstime);
      if (ret == ETIMEDOUT)
        break;
    }
    while (ret);
    ithread_mutex_unlock (&dlna->event_mutex);
    dlna_service_foreach (dlna->device, upnp_event_notify, dlna);
  }
  return NULL;
}

int
dlna_start (dlna_t *dlna)
{
  char *description = NULL;
  char *ip = NULL;
  int res;

  if (!dlna || !dlna->device || !dlna->device->get_description)
    return DLNA_ST_ERROR;

  description = dlna->device->get_description (dlna);
  if (!description)
    goto upnp_init_err;

  dlna_log (dlna, DLNA_MSG_INFO, "Initializing UPnP subsystem ...\n");

  ip = get_iface_address (dlna->interface);
  if (!ip)
    goto upnp_init_err;
  
  res = dlnaInit (ip, dlna->port);
  if (res != DLNA_E_SUCCESS)
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL,
              "Cannot initialize UPnP subsystem\n");
    goto upnp_init_err;
  }

  if (dlnaSetMaxContentLength (DLNA_MAX_CONTENT_LENGTH) != DLNA_E_SUCCESS)
    dlna_log (dlna, DLNA_MSG_ERROR, "Could not set UPnP max content length\n");

  dlna->port = dlnaGetServerPort ();
  dlna_log (dlna, DLNA_MSG_INFO, "UPnP device listening on %s:%d\n",
            dlnaGetServerIpAddress (), dlna->port);

  dlnaEnableWebserver (TRUE);

  res = dlnaSetVirtualDirCallbacks (&virtual_dir_callbacks, dlna);
  if (res != DLNA_E_SUCCESS)
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL,
              "Cannot set virtual directory callbacks\n");
    goto upnp_init_err;
  }
  
  res = dlnaAddVirtualDir (VIRTUAL_DIR);
  if (res != DLNA_E_SUCCESS)
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL,
              "Cannot add virtual directory for web server\n");
    goto upnp_init_err;
  }

  dlna_service_foreach (dlna->device, upnp_service_init, dlna);
  res = dlnaAddVirtualDir (SERVICES_VIRTUAL_DIR);
  if (res != DLNA_E_SUCCESS)
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL,
              "Cannot add virtual directory for services\n");
    goto upnp_init_err;
  }

  res = dlnaRegisterRootDevice2 (DLNAREG_BUF_DESC, description, 0, 1,
                                 device_callback_event_handler,
                                 dlna, &(dlna->dev));
  if (res != DLNA_E_SUCCESS)
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL, "Cannot register UPnP device\n");
    goto upnp_init_err;
  }

  res = dlnaUnRegisterRootDevice (dlna->dev);
  if (res != DLNA_E_SUCCESS)
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL, "Cannot unregister UPnP device\n");
    goto upnp_init_err;
  }

  res = dlnaRegisterRootDevice2 (DLNAREG_BUF_DESC, description, 0, 1,
                                 device_callback_event_handler,
                                 dlna, &(dlna->dev));
  if (res != DLNA_E_SUCCESS)
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL, "Cannot register UPnP device\n");
    goto upnp_init_err;
  }

  ithread_mutex_init (&dlna->event_mutex, NULL);
  ithread_cond_init (&dlna->eventing, NULL);
  ithread_create (&dlna->event_thread, NULL, dlna_event_thread, dlna);

  dlna_log (dlna, DLNA_MSG_INFO,
            "Sending UPnP advertisement for device ...\n");
  dlnaSendAdvertisement (dlna->dev, 1800);

  free (ip);
  free (description);
  return DLNA_ST_OK;

 upnp_init_err:
  if (ip)
    free (ip);
  if (description)
    free (description);
  return DLNA_ST_ERROR;
}

int
dlna_stop (dlna_t *dlna)
{
  if (!dlna)
    return DLNA_ST_ERROR;

  dlna_log (dlna, DLNA_MSG_INFO, "Stopping UPnP A/V Service ...\n");
  dlna->inited = 0;
  ithread_mutex_lock (&dlna->event_mutex);
  ithread_cond_signal (&dlna->eventing);
  ithread_mutex_unlock (&dlna->event_mutex);
  ithread_join (dlna->event_thread, NULL);
  ithread_mutex_destroy (&dlna->event_mutex);
  ithread_cond_destroy (&dlna->eventing);
  dlnaUnRegisterRootDevice (dlna->dev);
  dlnaFinish ();

  return DLNA_ST_OK;
}

int
upnp_add_response (upnp_action_event_t *ev, char *key, const char *value)
{
  char *val;
  int res;

  if (!ev || !ev->status || !key || !value)
    return 0;

  val = strdup (value);
  res = dlnaAddToActionResponse (&ev->ar->ActionResult,
                                 ev->ar->ActionName,
                                 ev->service->type, key, val);

  if (res != DLNA_E_SUCCESS)
  {
    free (val);
    return 0;
  }

  free (val);
  return 1;
}

char *
upnp_get_string (struct dlna_Action_Request *ar, const char *key)
{
  IXML_Node *node = NULL;

  if (!ar || !ar->ActionRequest || !key)
    return NULL;

  node = (IXML_Node *) ar->ActionRequest;
  if (!node)
    return NULL;

  node = ixmlNode_getFirstChild (node);
  if (!node)
    return NULL;

  node = ixmlNode_getFirstChild (node);
  for (; node; node = ixmlNode_getNextSibling (node))
    if (ixmlNode_getNodeName (node) &&
        !strcmp (ixmlNode_getNodeName (node), key))
    {
      node = ixmlNode_getFirstChild (node);
      if (!node)
        return strdup ("");
      return ixmlNode_getNodeValue (node) ?
        strdup (ixmlNode_getNodeValue (node)) : strdup ("");
    }

  return NULL;
}

int
upnp_get_ui4 (struct dlna_Action_Request *ar, const char *key)
{
  char *value;
  int val;

  if (!ar || !key)
    return 0;

  value = upnp_get_string (ar, key);
  if (!value && !strcmp (key, "ObjectID"))
    value = upnp_get_string (ar, "ContainerID");

  if (!value)
    return 0;

  val = atoi (value);
  free (value);

  return val;
}
