#include <iostream>
#include <liboslayer/os.hpp>
#include <liboslayer/Uuid.hpp>
#include <libupnp-tools/UPnPServer.hpp>
#include <libupnp-tools/HttpUtils.hpp>
#include <libupnp-tools/XmlUtils.hpp>
#include <libupnp-tools/UPnPActionInvoker.hpp>
#include <libupnp-tools/UPnPActionHandler.hpp>
#include <libupnp-tools/UPnPDeviceDeserializer.hpp>
#include <libupnp-tools/UPnPEventSubscriber.hpp>
#include <libupnp-tools/UPnPEventReceiver.hpp>
#include <liboslayer/XmlParser.hpp>
#include "utils.hpp"

using namespace std;
using namespace OS;
using namespace UPNP;
using namespace XML;
using namespace UTIL;
using namespace HTTP;

string dd(string uuid);
string scpd();

class MyActionHandler : public UPnPActionRequestHandler {
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

static UPnPNotify s_notify;

class MyNotifyHandler: public UPnPEventListener {
private:
public:
    MyNotifyHandler() {}
    virtual ~MyNotifyHandler() {}
	virtual void onNotify(UPnPNotify & notify) {
		s_notify = notify;
		cout << "NOTIFY" << endl;
		cout << " - SID: " << notify.sid() << endl;
		cout << " - SEQ: " << notify.seq() << endl;
		vector<string> names = notify.propertyNames();
		for (vector<string>::iterator iter = names.begin(); iter != names.end(); iter++) {
			string value = notify[*iter];
			cout << "  ** " << *iter << " := " << value << endl;
		}
	}
};


static void test_device_profile() {

	UuidGeneratorVersion1 gen;
	string uuid = gen.generate();

	UPnPEventReceiverConfig notiConfig(9998);
	UPnPEventReceiver notiServer(notiConfig);
	notiServer.addEventListener(AutoRef<UPnPEventListener>(new MyNotifyHandler));
	notiServer.startAsync();

	UPnPServer server(UPnPServer::Config(9001));

	UPnPDeviceProfile deviceProfile;
	deviceProfile.uuid() = uuid;
	deviceProfile.deviceDescription() = dd(uuid);
	UPnPServiceProfile serviceProfile;
	serviceProfile.scpd() = scpd();
	serviceProfile.serviceType() = "urn:schemas-dummy-com:service:Dummy:1";
	serviceProfile.scpdUrl() = "/scpd.xml?udn=" + uuid + "&serviceType=urn:schemas-dummy-com:service:Dummy:1";
	serviceProfile.controlUrl() = "/control?udn=" + uuid + "&serviceType=urn:schemas-dummy-com:service:Dummy:1";
	serviceProfile.eventSubUrl() = "/event?udn=" + uuid + "&serviceType=urn:schemas-dummy-com:service:Dummy:1";
	deviceProfile.serviceProfiles().push_back(serviceProfile);
	
	server.registerDeviceProfile(uuid, deviceProfile);
	server.getProfileManager().getDeviceProfileSessionByUuid(uuid)->setEnable(true);

	LinkedStringMap props;
	props["xxx"] = "";
	server.getPropertyManager().registerService(uuid, "urn:schemas-dummy-com:service:Dummy:1", props);

	AutoRef<UPnPActionRequestHandler> handler(new MyActionHandler);
	server.setActionRequestHandler(handler);

	ASSERT(server.parseTimeout("Second-300"), ==, 300000);

	server.startAsync();

	// profile search check
	{
		string scpdUrl = "/scpd.xml?udn=" + uuid + "&serviceType=urn:schemas-dummy-com:service:Dummy:1";
		UPnPDeviceProfile deviceProfile = server.getProfileManager().getDeviceProfileSessionHasScpdUrl(scpdUrl)->profile();
		UPnPServiceProfile service = deviceProfile.getServiceProfileByScpdUrl(scpdUrl);
		ASSERT(service.serviceType(), ==, "urn:schemas-dummy-com:service:Dummy:1");
		ASSERT(server.getProfileManager().hasDeviceProfileSessionByScpdUrl(scpdUrl), ==, true);
	}

	// dd check
	{
		string xml = HttpUtils::httpGet(Url("http://localhost:9001/device.xml?udn=" + uuid));
		ASSERT(xml, ==, dd(uuid));
	}

	// scpd check
	{
		string xml = HttpUtils::httpGet(Url("http://localhost:9001/scpd.xml?udn=" + uuid +
											"&serviceType=urn:schemas-dummy-com:service:Dummy:1"));
		ASSERT(xml, ==, scpd());
	}

	// scpd check
	{
		XML::XmlDocument doc = XML::DomParser::parse(scpd());
		ASSERT(doc.getRootNode().nil(), ==, false);

		vector<XmlNode*> actions = doc.getRootNode()->getElementsByTagName("action");
		for (vector<XmlNode*>::iterator iter = actions.begin(); iter != actions.end(); iter++) {
			UPnPDeviceDeserializer deserializer;
			UPnPAction action = deserializer.parseActionFromXmlNode(*iter);
			ASSERT(action.name(), ==, "GetProtocolInfo");
		}
		
		vector<XmlNode*> stateVariables = doc.getRootNode()->getElementsByTagName("stateVariable");
	}

	// parsing test
	{
		UPnPService service;
		UPnPDeviceDeserializer deserializer;
		UPnPScpd s = deserializer.parseScpdXml(scpd());
		ASSERT(s.hasStateVariable("SourceProtocolInfo"), ==, true);

		s.stateVariable("SourceProtocolInfo").addAllowedValue("hello");
		ASSERT(s.stateVariable("SourceProtocolInfo").hasAllowedValues(), ==, true);

		ASSERT(s.hasStateVariable("A_ARG_TYPE_BrowseFlag"), ==, true);
		ASSERT(s.stateVariable("A_ARG_TYPE_BrowseFlag").hasAllowedValues(), ==, true);
	}

	// action test
	{
		Url url = Url("http://localhost:9001/control?udn=" + uuid + "&serviceType=urn:schemas-dummy-com:service:Dummy:1");
		UPnPActionInvoker invoker(url);
		UPnPActionRequest request;
		request.serviceType() = "urn:schemas-dummy-com:service:Dummy:1";
		request.actionName() = "GetProtocolInfo";
		UPnPActionResponse response = invoker.invoke(request);
		ASSERT(response["Sink"], ==, "<sample sink>");
		ASSERT(response["Source"], ==, "<sample source>");
	}

	// event test
	{
		string serviceType = "urn:schemas-dummy-com:service:Dummy:1";
		Url url = Url("http://localhost:9001/event?udn=" + uuid + "&serviceType=" + serviceType);
		UPnPEventSubscriber subscriber(url);

		UPnPEventSubscribeRequest request("http://localhost:9998", 300);
		UPnPEventSubscribeResponse response = subscriber.subscribe(request);

		cout << " ** Recieved SID : " << response.sid() << endl;
		ASSERT(response.sid().empty(), ==, false);

		UPnPEventSubscription subscription(response.sid());
		subscription.udn() = uuid;
		subscription.serviceType() = serviceType;
		notiServer.addSubscription(subscription);

		idle(1000);

		ASSERT(s_notify.sid(), ==, response.sid());
	}

	notiServer.stop();
	server.stop();
}

int main(int argc, char *args[]) {

	test_device_profile();
    
    return 0;
}

string dd(string uuid) {
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
		"<UDN>uuid:" + uuid + "</UDN>"
		"<serviceList>"
		"<service>"
		"<serviceType>urn:schemas-dummy-com:service:Dummy:1</serviceType>"
		"<serviceId>urn:dummy-com:serviceId:dummy1</serviceId>"
		"<controlURL>/control?udn=" + uuid + "&amp;serviceType=" + dummy + "</controlURL>"
		"<eventSubURL>/event?udn=" + uuid + "&amp;serviceType=" + dummy + "</eventSubURL>"
		"<SCPDURL>/scpd.xml?udn=" + uuid + "&amp;serviceType=" + dummy + "</SCPDURL>"
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
