#include <iostream>
#include <string>
#include <map>
#include <liboslayer/Text.hpp>
#include <libupnp-tools/UPnPServer.hpp>
#include <libupnp-tools/UPnPDeviceXmlWriter.hpp>
#include <libupnp-tools/XmlDocumentPrinter.hpp>
#include <libupnp-tools/UPnPService.hpp>

using namespace std;
using namespace UTIL;
using namespace UPNP;
using namespace XML;

UPnPDevice makeDevice() {
	UPnPDevice device;
    device.setUdn("uuid:fc4ec57e-b051-11db-88f8-006008abcdef");
    device.setFriendlyName("Dummy Device");
	device["deviceType"] = "urn:schemas-upnp-org:device:MediaServer:1";
    device["manufacturer"] = "Plex, Inc.";
    device["manufacturerURL"] = "http://www.plexapp.com/";
    device["modelDescription"] = "Plex Media Server";
    device["modelName"] = "Plex Media Server";
    device["modelURL"] = "http://www.plexapp.com/";
    device["modelNumber"] = "0.9.12.8";
    device["serialNumber"] = "";
    device["dlna:X_DLNADOC"] = "DMS-1.50";
	device["dlna:X_DLNADOC"].setProperty("xmlns:dlna", "urn:schemas-dlna-org:device-1-0");

	UPnPService service;

    service["serviceType"] = "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1";
    service["serviceId"] = "urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar";
    service["SCPDURL"] = "/X_MS_MediaReceiverRegistrar/a94137b1-a05d-bd90-82d9-43fb458e5c49/scpd.xml";
    service["controlURL"] = "/X_MS_MediaReceiverRegistrar/a94137b1-a05d-bd90-82d9-43fb458e5c49/control.xml";
    service["eventSubURL"] = "/X_MS_MediaReceiverRegistrar/a94137b1-a05d-bd90-82d9-43fb458e5c49/event.xml";

	device.addService(service);
     
    service["serviceType"] = "urn:schemas-upnp-org:service:ContentDirectory:1";
    service["serviceId"] = "urn:upnp-org:serviceId:ContentDirectory";
    service["SCPDURL"] = "/ContentDirectory/a94137b1-a05d-bd90-82d9-43fb458e5c49/scpd.xml";
    service["controlURL"] = "/ContentDirectory/a94137b1-a05d-bd90-82d9-43fb458e5c49/control.xml";
    service["eventSubURL"] = "/ContentDirectory/a94137b1-a05d-bd90-82d9-43fb458e5c49/event.xml";

	device.addService(service);
     
    service["serviceType"] = "urn:schemas-upnp-org:service:ConnectionManager:1";
    service["serviceId"] = "urn:upnp-org:serviceId:ConnectionManager";
    service["SCPDURL"] = "/ConnectionManager/a94137b1-a05d-bd90-82d9-43fb458e5c49/scpd.xml";
    service["controlURL"] = "/ConnectionManager/a94137b1-a05d-bd90-82d9-43fb458e5c49/control.xml";
    service["eventSubURL"] = "/ConnectionManager/a94137b1-a05d-bd90-82d9-43fb458e5c49/event.xml";

	device.addService(service);

	return device;
}

void print_upnp_device(UPnPDevice & device) {
	XmlDocument doc = UPnPDeviceXmlWriter::makeDeviceDescriptionXmlDocument(device);
    XmlPrinter printer;
	printer.setShowPrologue(true);
    printer.setFormatted(true);
    string dump = printer.printDocument(doc);
    cout << dump << endl;
}

void test_has(const UPnPDevice & device, const string & name, const string & value) {
	bool has = device.hasServiceWithPropertyRecursive(name, value);
	cout << name << "::" << value << " > has: " << (has ? "Y" : "N") << endl;
}

int main(int argc, char * args[]) {

	UPnPDevice device = makeDevice();

	print_upnp_device(device);
	test_has(device, "serviceType", "urn:schemas-upnp-org:service:ConnectionManager:1");
	test_has(device, "serviceType", "urn:schemas-upnp-org:service:ContentDirectory:1");
	test_has(device, "serviceType", "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1");
	test_has(device, "serviceType", "urn:schemas-upnp-org:service:AVTransport:1");
	

	getchar();

	return 0;
}