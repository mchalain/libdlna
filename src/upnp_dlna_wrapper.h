#ifndef __UPNP_DLNA_WRAPPER_H__
#define __UPNP_DLNA_WRAPPER_H__

#define dlnaDevice_Handle           UpnpDevice_Handle
#define dlnaVirtualDirCallbacks     UpnpVirtualDirCallbacks
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

int dlnaSetVirtualDirCallbacks(
    struct dlnaVirtualDirCallbacks *callbacks,
    void *cookie);

#endif
