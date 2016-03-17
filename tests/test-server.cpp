#include <iostream>
#include <libupnp-tools/UPnPServer.hpp>
#include <libupnp-tools/Uuid.hpp>
#include <libupnp-tools/HttpUtils.hpp>
#include <libupnp-tools/XmlUtils.hpp>
#include <libupnp-tools/UPnPSession.hpp>
#include <liboslayer/XmlParser.hpp>
#include "utils.hpp"

using namespace std;
using namespace UPNP;
using namespace XML;
using namespace UTIL;
using namespace HTTP;

string dd(string udn);
string scpd();

static void test_device_profile() {
	
	string udn = Uuid::generateUuid();

	UPnPServerProfile profile;
	profile["listen.port"] = "9001";

	UPnPServer server(profile);

	UPnPDeviceProfile device;
	device.udn() = udn;
	device.deviceDescription() = dd(udn);
	device.scpd("urn:schemas-dummy-com:service:Dummy:1") = scpd();
	UPnPService service;
	service["serviceType"] = "urn:schemas-dummy-com:service:Dummy:1";
	service["SCPDURL"] = "scpd.xml?udn=" + udn + "&serviceType=urn:schemas-dummy-com:service:Dummy:1";
	device.services().push_back(service);
	server[udn] = device;

	server.startAsync();

	{
		string scpdUrl = "scpd.xml?udn=" + udn + "&serviceType=urn:schemas-dummy-com:service:Dummy:1";
		UPnPDeviceProfile deviceProfile = server.getDeviceProfileHasScpdUrl(scpdUrl);
		UPnPService service = deviceProfile.getServiceWithScpdUrl(scpdUrl);
		ASSERT(service.getServiceType(), ==, "urn:schemas-dummy-com:service:Dummy:1");

		ASSERT(server.hasDeviceProfileWithScpdUrl(scpdUrl), ==, true);
	}

	{
		string xml = HttpUtils::httpGet(Url("http://localhost:9001/device.xml?udn=" + udn));
		ASSERT(xml, ==, dd(udn));
	}

	{
		string xml = HttpUtils::httpGet(Url("http://localhost:9001/scpd.xml?udn=" + udn +
											"&serviceType=urn:schemas-dummy-com:service:Dummy:1"));
		ASSERT(xml, ==, scpd());
	}

	{
		XML::XmlDocument doc = XML::DomParser::parse(scpd());
		ASSERT(doc.getRootNode().nil(), ==, false);

		UPnPSession session(udn);

		vector<XmlNode*> actions = doc.getRootNode()->getElementsByTagName("action");
		for (vector<XmlNode*>::iterator iter = actions.begin(); iter != actions.end(); iter++) {
			UPnPAction action = session.parseActionFromActionXml(*iter);
			ASSERT(action.name(), ==, "GetProtocolInfo");
		}
		
		vector<XmlNode*> stateVariables = doc.getRootNode()->getElementsByTagName("stateVariable");
	}

	server.stop();
}

int main(int argc, char *args[]) {

	test_device_profile();
    
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
