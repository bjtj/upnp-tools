#include <iostream>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <libupnp-tools/Uuid.hpp>
#include <libupnp-tools/UPnPModels.hpp>
#include <libupnp-tools/SSDPMsearchSender.hpp>
#include <libupnp-tools/NetworkUtil.hpp>
#include <libupnp-tools/UPnPServer.hpp>

using namespace std;
using namespace SSDP;
using namespace HTTP;
using namespace UPNP;

size_t readline(char * buffer, size_t max) {
    if (fgets(buffer, (int)max - 1, stdin)) {
		buffer[strlen(buffer) - 1] = 0;
		return strlen(buffer);
	}
    return 0;
}


string dd(string udn) {
	string dummy = "urn:schemas-dummy-com:service:Dummy:1";
	string xml = "<?xml version=\"1.0\"?>"
		"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
		"<specVersion>"
		"<major>1</major>"
		"<minor>0</minor>"
		"</specVersion><device>"
		"<deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType>"
		"<friendlyName>UPnP Test Device</friendlyName>"
		"<manufacturer>Testers</manufacturer>"
		"<manufacturerURL>www.example.com</manufacturerURL>"
		"<modelDescription>UPnP Test Device</modelDescription>"
		"<modelName>UPnP Test Device</modelName>"
		"<modelNumber>1</modelNumber>"
		"<modelURL>www.example.com</modelURL>"
		"<serialNumber>12345678</serialNumber>"
		"<UDN>uuid:" + udn + "</UDN>"
		"<serviceList>"
		"<service>"
		"<serviceType>urn:schemas-dummy-com:service:Dummy:1</serviceType>"
		"<serviceId>urn:dummy-com:serviceId:dummy1</serviceId>"
		"<controlURL>control?udn=" + udn + "&serviceType=" + dummy + "</controlURL>"
		"<eventSubURL>event?udn=" + udn + "&serviceType=" + dummy + "</eventSubURL>"
		"<SCPDURL>scpd.xml?udn=" + udn + "&serviceType=" + dummy + "</SCPDURL>"
		"</service></serviceList>"
		"</root>";

	return xml;
}

string scpd() {

	string xml = "<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">"
		"<specVersion>"
		"<major>1</major>"
		"<minor>0</minor>"
		"</specVersion>"
		"<actionList>"
		"<action><name>GetProtocolInfo</name><argumentList><argument><name>Source</name><direction>out</direction><relatedStateVariable>SourceProtocolInfo</relatedStateVariable></argument><argument><name>Sink</name><direction>out</direction><relatedStateVariable>SinkProtocolInfo</relatedStateVariable></argument></argumentList></action>"
		"</actionList>"
		"<serviceStateTable>"
		"<stateVariable sendEvents=\"yes\"><name>SourceProtocolInfo</name><dataType>string</dataType></stateVariable><stateVariable sendEvents=\"yes\"><name>SinkProtocolInfo</name><dataType>string</dataType></stateVariable>"
		"</serviceStateTable>"
		"</scpd>";

	return xml;
}

void notifyAlive(const string & udn, const string & deviceType, const string & location) {

	string ssdp = "NOTIFY * HTTP/1.1\r\n"
		"Cache-Control: max-age=120\r\n"
		"HOST: 239.255.255.250:1900\r\n"
		"Location: " + location + "\r\n"
		"NT: " + deviceType + "\r\n"
		"NTS: ssdp:alive\r\n"
		"Server: Net-OS 5.xx UPnP/1.0\r\n"
		"USN: uuid:" + udn + "::" + deviceType + "\r\n"
		"\r\n";

	SSDPMsearchSender sender;
	sender.sendMcastToAllInterfaces(ssdp, "239.255.255.250", 1900);
	sender.close();
}

void notifyByeBye(const string & udn, const string & deviceType) {
	string ssdp = "NOTIFY * HTTP/1.1\r\n"
		"Host: 239.255.255.250:1900\r\n"
		"NT: " + (deviceType.empty() ? "uuid:" + udn : deviceType)  + "\r\n"
		"NTS: ssdp:byebye\r\n"
		"USN: uuid:" + udn + (deviceType.empty() ? "" : "::" + deviceType) + "\r\n"
		"\r\n";

	SSDPMsearchSender sender;
	sender.sendMcastToAllInterfaces(ssdp, "239.255.255.250", 1900);
	sender.close();
}

int main(int argc, char *args[]) {

	string udn = Uuid::generateUuid();

	UPnPServerProfile profile;
	profile["listen.port"] = "9001";

	UPnPServer server(profile);
	server.startAsync();

	UPnPDeviceProfile device;
	device.udn() = udn;
	device.deviceDescription() = dd(udn);
	device.scpd("urn:schemas-dummy-com:service:Dummy:1") = scpd();
	UPnPService service;
	service["serviceType"] = "urn:schemas-dummy-com:service:Dummy:1";
	service["SCPDURL"] = "scpd.xml?udn=" + udn + "&serviceType=urn:schemas-dummy-com:service:Dummy:1";
	device.services().push_back(service);
	server[udn] = device;

	cout << "udn: " << udn << endl;

	while (1) {
		char buffer[1024] = {0,};
		if (readline(buffer, sizeof(buffer)) > 0) {
			if (!strcmp(buffer, "q")) {
				break;
			} else if (!strcmp(buffer, "alive")) {
				server.notifyAliveWithDeviceType(device, "upnp:rootdevice");
			} else if (!strcmp(buffer, "byebye")) {
				server.notifyByeByeWithDeviceType(device, "upnp:rootdevice");
			}
		}
	}

	server.stop();
    
    return 0;
}
