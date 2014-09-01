#ifndef DEVICES_H
#define DEVICES_H

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

#define DLNA_DLNADOC_DESCRIPTION \
"    <dlna:X_DLNADOC xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">DMR-1.00</dlna:X_DLNADOC>"

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
