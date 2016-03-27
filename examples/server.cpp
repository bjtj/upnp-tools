#include <iostream>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <libupnp-tools/Uuid.hpp>
#include <libupnp-tools/UPnPModels.hpp>
#include <libupnp-tools/SSDPMsearchSender.hpp>
#include <libupnp-tools/NetworkUtil.hpp>
#include <libupnp-tools/UPnPServer.hpp>
#include <libupnp-tools/UPnPActionHandler.hpp>

using namespace std;
using namespace SSDP;
using namespace HTTP;
using namespace UPNP;
using namespace UTIL;

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
		"<controlURL>/control?udn=" + udn + "&serviceType=" + dummy + "</controlURL>"
		"<eventSubURL>/event?udn=" + udn + "&serviceType=" + dummy + "</eventSubURL>"
		"<SCPDURL>/scpd.xml?udn=" + udn + "&serviceType=" + dummy + "</SCPDURL>"
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

class MyActionHandler : public UPnPActionHandler {
private:
public:
    MyActionHandler() {}
    virtual ~MyActionHandler() {}

	virtual void handleActionRequest(UPnPActionRequest & request, UPnPActionResponse & response) {

		cout << "** Action requst **" << endl;
		cout << " - service type : " << request.serviceType() << endl;
		cout << " - action name : " << request.actionName() << endl;
		
		if (request.actionName() == "GetProtocolInfo") {
			response["Source"] = "<sample source>";
			response["Sink"] = "<sample sink>";
		}
	}
};

int main(int argc, char *args[]) {

	UuidGeneratorDefault gen;
	string udn = gen.generate();

	UPnPServerConfig config(9001);
	UPnPServer server(config);
	server.startAsync();

	UPnPDeviceProfile deviceProfile;
	deviceProfile.udn() = udn;
	deviceProfile.deviceDescription() = dd(udn);
	UPnPServiceProfile serviceProfile;
	serviceProfile.scpd() = scpd();
	serviceProfile.serviceType() = "urn:schemas-dummy-com:service:Dummy:1";
	serviceProfile.scpdUrl() = "/scpd.xml?udn=" + udn + "&serviceType=urn:schemas-dummy-com:service:Dummy:1";
	serviceProfile.controlUrl() = "/control?udn=" + udn + "&serviceType=urn:schemas-dummy-com:service:Dummy:1";
	serviceProfile.eventSubUrl() = "/event?udn=" + udn + "&serviceType=urn:schemas-dummy-com:service:Dummy:1";
	deviceProfile.serviceProfiles().push_back(serviceProfile);
	server[udn] = deviceProfile;

	AutoRef<UPnPActionHandler> handler(new MyActionHandler);
	server.setActionHandler(handler);

	cout << "udn: " << udn << endl;

	while (1) {
		char buffer[1024] = {0,};
		if (readline(buffer, sizeof(buffer)) > 0) {
			if (!strcmp(buffer, "q")) {
				break;
			} else if (!strcmp(buffer, "alive")) {
				server.notifyAliveWithDeviceType(deviceProfile, "upnp:rootdevice");
			} else if (!strcmp(buffer, "byebye")) {
				server.notifyByeByeWithDeviceType(deviceProfile, "upnp:rootdevice");
			}
		}
	}

	server.stop();
    
    return 0;
}
