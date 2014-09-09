/*
 * libdlna: reference DLNA standards implementation.
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

#ifndef __UPNP_DLNA_WRAPPER_H__
#define __UPNP_DLNA_WRAPPER_H__

#define dlnaDevice_Handle           UpnpDevice_Handle
#define dlnaVirtualDirCallbacks    UpnpVirtualDirCallbacks
#define dlna_Action_Request         Upnp_Action_Request
#define dlna_State_Var_Request      Upnp_State_Var_Request
#define dlna_Subscription_Request  Upnp_Subscription_Request
#define dlna_EventType              Upnp_EventType
#define dlnaWebFileHandle           UpnpWebFileHandle
#define dlnaOpenFileMode           UpnpOpenFileMode

#define DLNA_E_SUCCESS              UPNP_E_SUCCESS
#define DLNA_SOAP_E_INVALID_ACTION  UPNP_SOAP_E_INVALID_ACTION
#define DLNAREG_BUF_DESC            UPNPREG_BUF_DESC
#define DLNA_CONTROL_ACTION_REQUEST UPNP_CONTROL_ACTION_REQUEST
#define DLNA_CONTROL_ACTION_COMPLETE    UPNP_CONTROL_ACTION_COMPLETE
#define DLNA_EVENT_SUBSCRIPTION_REQUEST UPNP_EVENT_SUBSCRIPTION_REQUEST
#define DLNA_CONTROL_GET_VAR_REQUEST    UPNP_CONTROL_GET_VAR_REQUEST
#define DLNA_READ                   UPNP_READ

#define dlnaInit UpnpInit
#define dlnaSetMaxContentLength     UpnpSetMaxContentLength
#define dlnaGetServerPort           UpnpGetServerPort
#define dlnaGetServerIpAddress      UpnpGetServerIpAddress
#define dlnaGetServerIpAddress      UpnpGetServerIpAddress
#define dlnaEnableWebserver         UpnpEnableWebserver
#define dlnaAddVirtualDir           UpnpAddVirtualDir
#define dlnaRegisterRootDevice2     UpnpRegisterRootDevice2
#define dlnaUnRegisterRootDevice    UpnpUnRegisterRootDevice
#define dlnaSendAdvertisement       UpnpSendAdvertisement
#define dlnaAddToActionResponse     UpnpAddToActionResponse
#define dlnaFinish                  UpnpFinish
#define dlnaAcceptSubscriptionExt   UpnpAcceptSubscriptionExt
#define dlnaAddToPropertySet        UpnpAddToPropertySet
#define dlnaGetErrorMessage         UpnpGetErrorMessage
#define dlnaNotifyExt               UpnpNotifyExt

int dlnaSetVirtualDirCallbacks(
    struct dlnaVirtualDirCallbacks *callbacks,
    void *cookie);

#endif
