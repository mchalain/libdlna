#ifndef DEVICES_H
#define DEVICES_H

typedef struct dlna_device_s dlna_device_t;

struct dlna_device_s {
  /* UPnP Services */
  upnp_service_t *services;

  char *(*get_description) (dlna_t *);

  char *urn_type;
  char *friendly_name;
  char *manufacturer;
  char *manufacturer_url;
  char *model_description;
  char *model_name;
  char *model_number;
  char *model_url;
  char *serial_number;
  char *uuid;
  char *presentation_url;
};

dlna_device_t *dlna_device_new ();

#define DLNA_DESCRIPTION_HEADER \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">" \
"  <specVersion>" \
"    <major>1</major>" \
"    <minor>0</minor>" \
"  </specVersion>" \
"  <device>" \
"    <deviceType>%s</deviceType>" \
"    <friendlyName>%s: 1</friendlyName>" \
"    <manufacturer>%s</manufacturer>" \
"    <manufacturerURL>%s</manufacturerURL>" \
"    <modelDescription>%s</modelDescription>" \
"    <modelName>%s</modelName>" \
"    <modelNumber>%s</modelNumber>" \
"    <modelURL>%s</modelURL>" \
"    <serialNumber>%s</serialNumber>" \
"    <UDN>uuid:%s</UDN>"

#define DLNA_DEVICE_PRESENTATION \
"    <presentationURL>%s</presentationURL>"

#define DLNA_DLNADOC_DMR_DESCRIPTION \
"    <dlna:X_DLNADOC xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">DMR-1.00</dlna:X_DLNADOC>"

#define DLNA_DLNADOC_DMS_DESCRIPTION \
"    <dlna:X_DLNADOC xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">DMS-1.00</dlna:X_DLNADOC>"

#define DLNA_ICONLIST_HEADER \
"    <iconList>"

#define DLNA_ICONLIST_FOOTER \
"    </iconList>"

#define DLNA_SERVICELIST_HEADER \
"    <serviceList>"

#define DLNA_SERVICELIST_FOOTER \
"    </serviceList>"

#define DLNA_DESCRIPTION_FOOTER \
"  </device>" \
"</root>"

#define DLNA_SERVICE_DESCRIPTION \
"      <service>" \
"        <serviceType>%s</serviceType>" \
"        <serviceId>%s</serviceId>" \
"        <SCPDURL>%s/%s</SCPDURL>" \
"        <controlURL>%s/%s</controlURL>" \
"        <eventSubURL>%s/%s</eventSubURL>" \
"      </service>" \

#endif
