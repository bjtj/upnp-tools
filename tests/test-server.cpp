#include <iostream>
#include <libupnp-tools/UPnPServer.hpp>
#include <libupnp-tools/Uuid.hpp>
#include <libupnp-tools/HttpUtils.hpp>
#include <libupnp-tools/XmlUtils.hpp>
#include <libupnp-tools/UPnPSession.hpp>
#include <libupnp-tools/UPnPActionInvoker.hpp>
#include <libupnp-tools/UPnPActionHandler.hpp>
#include <liboslayer/XmlParser.hpp>
#include "utils.hpp"

using namespace std;
using namespace UPNP;
using namespace XML;
using namespace UTIL;
using namespace HTTP;

string dd(string udn);
string scpd();

class MyActionHandler : public UPnPActionHandler {
private:
public:
    MyActionHandler() {}
    virtual ~MyActionHandler() {}

	virtual void handleActionRequest(UPnPActionRequest & request, UPnPActionResponse & response) {
		if (request.actionName() == "GetProtocolInfo") {
			response["Source"] = "<sample source>";
			response["Sink"] = "<sample sink>";
		}
	}
};


static void test_device_profile() {
	
	string udn = Uuid::generateUuid();

	UPnPServerProfile profile;
	profile["listen.port"] = "9001";

	UPnPServer server(profile);

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
	
	server.startAsync();

	// profile search check
	{
		string scpdUrl = "/scpd.xml?udn=" + udn + "&serviceType=urn:schemas-dummy-com:service:Dummy:1";
		UPnPDeviceProfile deviceProfile = server.getDeviceProfileHasScpdUrl(scpdUrl);
		UPnPServiceProfile service = deviceProfile.getServiceProfileWithScpdUrl(scpdUrl);
		ASSERT(service.serviceType(), ==, "urn:schemas-dummy-com:service:Dummy:1");
		ASSERT(server.hasDeviceProfileWithScpdUrl(scpdUrl), ==, true);
	}

	// dd check
	{
		string xml = HttpUtils::httpGet(Url("http://localhost:9001/device.xml?udn=" + udn));
		ASSERT(xml, ==, dd(udn));
	}

	// scpd check
	{
		string xml = HttpUtils::httpGet(Url("http://localhost:9001/scpd.xml?udn=" + udn +
											"&serviceType=urn:schemas-dummy-com:service:Dummy:1"));
		ASSERT(xml, ==, scpd());
	}

	// scpd check
	{
		XML::XmlDocument doc = XML::DomParser::parse(scpd());
		ASSERT(doc.getRootNode().nil(), ==, false);

		UPnPSession session(udn);

		vector<XmlNode*> actions = doc.getRootNode()->getElementsByTagName("action");
		for (vector<XmlNode*>::iterator iter = actions.begin(); iter != actions.end(); iter++) {
			UPnPAction action = session.parseActionFromXml(*iter);
			ASSERT(action.name(), ==, "GetProtocolInfo");
		}
		
		vector<XmlNode*> stateVariables = doc.getRootNode()->getElementsByTagName("stateVariable");
	}

	// parse test
	{
		UPnPSession session(udn);
		UPnPService service;
		session.parseScpdFromXml(service, scpd());
		UPnPStateVariable stateVariable = service.getStateVariable("SourceProtocolInfo");
		ASSERT(stateVariable.name(), ==, "SourceProtocolInfo");

		stateVariable.addAllowedValue("hello");
		ASSERT(stateVariable.hasAllowedValues(), ==, true);

		stateVariable = service.getStateVariable("A_ARG_TYPE_BrowseFlag");
		ASSERT(stateVariable.name(), ==, "A_ARG_TYPE_BrowseFlag");
		ASSERT(stateVariable.hasAllowedValues(), ==, true);
	}

	// send action check
	{
		Url url = Url("http://localhost:9001/control?udn=" + udn + "&serviceType=urn:schemas-dummy-com:service:Dummy:1");
		UPnPActionInvoker invoker(url);
		UPnPActionRequest request;
		request.serviceType() = "urn:schemas-dummy-com:service:Dummy:1";
		request.actionName() = "GetProtocolInfo";
		UPnPActionResponse response = invoker.invoke(request);
		ASSERT(response["Sink"], ==, "<sample sink>");
		ASSERT(response["Source"], ==, "<sample source>");
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
		"<stateVariable sendEvents=\"yes\">"
		"<name>SourceProtocolInfo</name>"
		"<dataType>string</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"yes\">"
		"<name>SinkProtocolInfo</name>"
		"<dataType>string</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_BrowseFlag</name>"
		"<dataType>string</dataType>"
		"<allowedValueList>"
		"<allowedValue>BrowseMetadata</allowedValue>"
		"<allowedValue>BrowseDirectChildren</allowedValue>"
		"</allowedValueList>"
		"</stateVariable>"
		"</serviceStateTable>"
		"</scpd>";

	return xml;
}
