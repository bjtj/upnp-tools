#include <iostream>
#include <liboslayer/os.hpp>
#include <liboslayer/Uuid.hpp>
#include <liboslayer/XmlParser.hpp>
#include <liboslayer/XmlEncoderDecoder.hpp>
#include <liboslayer/Logger.hpp>
#include <libupnp-tools/UPnPServer.hpp>
#include <libupnp-tools/HttpUtils.hpp>
#include <libupnp-tools/XmlUtils.hpp>
#include <libupnp-tools/UPnPActionInvoker.hpp>
#include <libupnp-tools/UPnPDeviceDeserializer.hpp>
#include <libupnp-tools/UPnPEventSubscriber.hpp>
#include <libupnp-tools/UPnPEventReceiver.hpp>
#include <libupnp-tools/UPnPTerms.hpp>
#include "utils.hpp"

using namespace std;
using namespace OS;
using namespace UPNP;
using namespace XML;
using namespace UTIL;
using namespace HTTP;

string dd(const UDN & udn);
string scpd();

static string scpd_url(const UDN & udn, const string & serviceType) {
	return ("/scpd.xml?udn=" + udn.toString() + "&serviceType=" + serviceType);
}

static string control_url(const UDN & udn, const string & serviceType) {
	return ("/control.xml?udn=" + udn.toString() + "&serviceType=" + serviceType);
}

static string event_url(const UDN & udn, const string & serviceType) {
	return ("/event.xml?udn=" + udn.toString() + "&serviceType=" + serviceType);
}

class MyActionHandler : public UPnPActionRequestHandler {
private:
public:
    MyActionHandler() {}
    virtual ~MyActionHandler() {}

	virtual bool handleActionRequest(UPnPActionRequest & request, UPnPActionResponse & response) {
		if (request.actionName() == "GetProtocolInfo") {
			response["Source"] = "<sample source>";
			response["Sink"] = "<sample sink>";
			return true;
		}
		return false;
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
	UDN udn("uuid:" + uuid);
	string serviceType = "urn:schemas-dummy-com:service:Dummy:1";

	UPnPEventReceiverConfig notiConfig(9998);
	UPnPEventReceiver notiServer(notiConfig);
	notiServer.addEventListener(AutoRef<UPnPEventListener>(new MyNotifyHandler));
	notiServer.startAsync();

	UPnPServer server(UPnPServer::Config(9001));
	server.setDebug(AutoRef<UPnPDebug>(new UPnPDebug));

	AutoRef<UPnPDeviceProfile> deviceProfile(
		new UPnPDeviceProfile(UPnPDeviceDeserializer::deserializeDevice(dd(udn))));
	deviceProfile->setUdn(udn);
	deviceProfile->device()->setScpdUrl("/scpd.xml?udn=$udn&serviceType=$serviceType");
	deviceProfile->device()->setControlUrl("/control.xml?udn=$udn&serviceType=$serviceType");
	deviceProfile->device()->setEventSubUrl("/event.xml?udn=$udn&serviceType=$serviceType");

	AutoRef<UPnPService> service = deviceProfile->device()->getService(serviceType);
	service->scpd() = UPnPDeviceDeserializer::deserializeScpd(scpd());

	server.registerDeviceProfile(deviceProfile);
	server.getProfileManager().getDeviceProfile(udn)->enabled() = true;

	LinkedStringMap props;
	props["xxx"] = "";
	server.getPropertyManager().registerService(udn, serviceType, props);

	AutoRef<UPnPActionRequestHandler> handler(new MyActionHandler);
	server.setActionRequestHandler(handler);

	ASSERT(Second::fromString("Second-300").milli(), ==, 300000);

	server.startAsync();

	// profile search check
	{
		string scpdUrl = scpd_url(udn, serviceType);;
		cout << "scpd url: " << scpdUrl << endl;
		AutoRef<UPnPDeviceProfile> deviceProfile =
			server.getProfileManager().getDeviceProfileHasScpdUrl(scpdUrl);
		cout << "get service - " << scpdUrl << endl;
		AutoRef<UPnPService> service = deviceProfile->device()->getServiceWithScpdUrl(scpdUrl);
		ASSERT(service->serviceType(), ==, serviceType);
		try {
			server.getProfileManager().getDeviceProfileHasScpdUrl(scpdUrl);
		} catch (...) {
			ASSERT(true, ==, false);
		}
		
	}

	// dd check
	{
		Url url("http://127.0.0.1:9001/device.xml?udn=" + udn.toString());
		string ddXml = HttpUtils::httpGet(url);
		ASSERT(ddXml, ==, dd(udn));
	}

	// scpd check
	{
		Url url("http://127.0.0.1:9001" + scpd_url(udn, serviceType));
		string scpdXml = HttpUtils::httpGet(url);
		ASSERT(scpdXml, ==, scpd());
	}

	// scpd check
	{
		XML::XmlDocument doc = XML::DomParser::parse(scpd());
		ASSERT(doc.getRootNode().nil(), ==, false);
		vector<AutoRef<XmlNode> > actions = doc.getRootNode()->getElementsByTagName("action");
		for (vector<AutoRef<XmlNode> >::iterator iter = actions.begin();
			 iter != actions.end(); iter++)
		{
			UPnPAction action = UPnPDeviceDeserializer::deserializeActionNode(*iter);
			ASSERT(action.name(), ==, "GetProtocolInfo");
		}
		vector<AutoRef<XmlNode> > stateVariables =
			doc.getRootNode()->getElementsByTagName("stateVariable");
	}

	// parsing test
	{
		UPnPService service;
		UPnPScpd s = UPnPDeviceDeserializer::deserializeScpd(scpd());
		ASSERT(s.hasStateVariable("SourceProtocolInfo"), ==, true);
		s.stateVariable("SourceProtocolInfo").addAllowedValue("hello");
		ASSERT(s.stateVariable("SourceProtocolInfo").hasAllowedValues(), ==, true);
		ASSERT(s.hasStateVariable("A_ARG_TYPE_BrowseFlag"), ==, true);
		ASSERT(s.stateVariable("A_ARG_TYPE_BrowseFlag").hasAllowedValues(), ==, true);
	}

	// action test
	{
		Url url = Url("http://127.0.0.1:9001" + control_url(udn, serviceType));
		UPnPActionInvoker invoker(url);
		UPnPActionRequest request;
		request.serviceType() = serviceType;
		request.actionName() = "GetProtocolInfo";
		UPnPActionResponse response = invoker.invoke(request);
		ASSERT(response["Sink"], ==, "<sample sink>");
		ASSERT(response["Source"], ==, "<sample source>");
	}

	// event test
	{
		Url url = Url("http://127.0.0.1:9001" + event_url(udn, serviceType));
		UPnPEventSubscriber subscriber(url);
		UPnPEventSubscribeRequest request("http://127.0.0.1:9998", 300);
		UPnPEventSubscribeResponse response = subscriber.subscribe(request);
		cout << " ** Recieved SID : " << response.sid() << endl;
		ASSERT(response.sid().empty(), ==, false);
		UPnPEventSubscription subscription(response.sid());
		subscription.udn() = udn;
		subscription.serviceType() = serviceType;
		notiServer.addSubscription(subscription);
		idle(1000);
		ASSERT(s_notify.sid(), ==, response.sid());
	}

	notiServer.stop();
	server.stop();
}

int main(int argc, char *args[]) {
	LoggerFactory::instance().setProfile("*", "basic", "console");
	test_device_profile();
    return 0;
}

string dd(const UDN & udn) {
	string dummy = "urn:schemas-dummy-com:service:Dummy:1";
	string xml = "<?xml version=\"1.0\"?>\r\n"
		"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\r\n"
		"<specVersion>\r\n"
		"<major>1</major>\r\n"
		"<minor>0</minor>\r\n"
		"</specVersion>\r\n"
		"<device>\r\n"
		"<deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType>\r\n"
		"<friendlyName>UPnP Test Device</friendlyName>\r\n"
		"<manufacturer>Testers</manufacturer>\r\n"
		"<manufacturerURL>www.example.com</manufacturerURL>\r\n"
		"<modelDescription>UPnP Test Device</modelDescription>\r\n"
		"<modelName>UPnP Test Device</modelName>\r\n"
		"<modelNumber>1</modelNumber>\r\n"
		"<modelURL>www.example.com</modelURL>\r\n"
		"<serialNumber>12345678</serialNumber>\r\n"
		"<UDN>" + udn.toString() + "</UDN>\r\n"
		"<serviceList>\r\n"
		"<service>\r\n"
		"<serviceType>urn:schemas-dummy-com:service:Dummy:1</serviceType>\r\n"
		"<serviceId>urn:dummy-com:serviceId:dummy1</serviceId>\r\n"
		"<SCPDURL>" + XmlEncoder::encode(scpd_url(udn, dummy)) + "</SCPDURL>\r\n"
		"<controlURL>" + XmlEncoder::encode(control_url(udn, dummy)) + "</controlURL>\r\n"
		"<eventSubURL>" + XmlEncoder::encode(event_url(udn, dummy)) + "</eventSubURL>\r\n"
		"</service>\r\n"
		"</serviceList>\r\n"
		"</device>\r\n"
		"</root>";

	return xml;
}

string scpd() {
	string xml = "<?xml version=\"1.0\"?>\r\n"
		"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">\r\n"
		"<specVersion>\r\n"
		"<major>1</major>\r\n"
		"<minor>0</minor>\r\n"
		"</specVersion>\r\n"
		"<actionList>\r\n"
		"<action>\r\n"
		"<name>GetProtocolInfo</name>\r\n"
		"<argumentList>\r\n"
		"<argument>\r\n"
		"<name>Source</name>\r\n"
		"<direction>out</direction>\r\n"
		"<relatedStateVariable>SourceProtocolInfo</relatedStateVariable>\r\n"
		"</argument>\r\n"
		"<argument>\r\n"
		"<name>Sink</name>\r\n"
		"<direction>out</direction>\r\n"
		"<relatedStateVariable>SinkProtocolInfo</relatedStateVariable>\r\n"
		"</argument>\r\n"
		"</argumentList>\r\n"
		"</action>\r\n"
		"</actionList>\r\n"
		"<serviceStateTable>\r\n"
		"<stateVariable sendEvents=\"yes\">\r\n"
		"<name>SourceProtocolInfo</name>\r\n"
		"<dataType>string</dataType>\r\n"
		"</stateVariable>\r\n"
		"<stateVariable sendEvents=\"yes\">\r\n"
		"<name>SinkProtocolInfo</name>\r\n"
		"<dataType>string</dataType>\r\n"
		"</stateVariable>\r\n"
		"<stateVariable sendEvents=\"no\">\r\n"
		"<name>A_ARG_TYPE_BrowseFlag</name>\r\n"
		"<dataType>string</dataType>\r\n"
		"<allowedValueList>\r\n"
		"<allowedValue>BrowseMetadata</allowedValue>\r\n"
		"<allowedValue>BrowseDirectChildren</allowedValue>\r\n"
		"</allowedValueList>\r\n"
		"</stateVariable>\r\n"
		"</serviceStateTable>\r\n"
		"</scpd>";

	return xml;
}
