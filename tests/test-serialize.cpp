#include "utils.hpp"
#include <liboslayer/FileStream.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <libhttp-server/StringDataSink.hpp>
#include <libupnp-tools/UPnPDeviceDeserializer.hpp>
#include <libupnp-tools/UPnPDeviceSerializer.hpp>
#include <libupnp-tools/Uuid.hpp>

using namespace std;
using namespace OS;
using namespace HTTP;
using namespace UTIL;
using namespace SSDP;
using namespace UPNP;

static string dd(string udn);
static string dd_fs(string udn);
static string scpd();

static string dummy = "urn:schemas-dummy-com:service:Dummy:1";

class RequestHandler : public HttpRequestHandler {
private:
	string _udn;
public:
    RequestHandler(const string & udn) {
		_udn = udn;
	}
    virtual ~RequestHandler() {}
	virtual AutoRef<DataSink> getDataSink() {
		return AutoRef<DataSink>(new StringDataSink);
	}
	virtual void onHttpRequestContentCompleted(HttpRequest & request, AutoRef<DataSink> sink, HttpResponse & response) {
		cout << " ** path : " << request.getHeader().getPart2() << endl;
		if (request.getPath() == "/device.xml") {
			response.setStatusCode(200);
			response.setContentType("text/xml");
			setFixedTransfer(response, dd(_udn));
		} else if (request.getHeader().getPart2() == "/scpd.xml?udn=" + _udn + "&serviceType=" + dummy) {
			response.setStatusCode(200);
			response.setContentType("text/xml");
			setFixedTransfer(response, scpd());
		} else {
			response.setStatusCode(404);
		}
	}
	string udn() {
		return _udn;
	}
	SSDPHeader getSSDPHeader() {
		OS::InetAddress addr;
		SSDPHeader header("NOTIFY * HTTP/1.1\r\n"
						  "HOST: 239.255.255.250:1900\r\n"
						  "Location: http://localhost:9998/device.xml\r\n"
						  "NTS: ssdp:alive\r\n"
						  "USN: uuid:" + _udn + "::rootdevice\r\n"
						  "\r\n", addr);
		return header;
	}
	
};

static void test_deserialize() {
	
	UuidGeneratorDefault gen;
	string udn = gen.generate();
	HttpServerConfig config(9999);
	AnotherHttpServer server(config);
	server.registerRequestHandler("/*", AutoRef<HttpRequestHandler>(new RequestHandler(udn)));
	server.startAsync();

	AutoRef<UPnPDevice> device = UPnPDeviceDeserializer::buildDevice(Url("http://localhost:9999/device.xml"));

	ASSERT(device->getUdn(), ==, "uuid:" + udn);
	ASSERT(device->getFriendlyName(), ==, "UPnP Test Device");

	ASSERT(device->hasService("urn:schemas-dummy-com:service:Dummy:1"), ==, true);
	ASSERT(device->getService("urn:schemas-dummy-com:service:Dummy:1")->getAction("GetProtocolInfo").arguments().size(), ==, 2);

	server.stop();
}

static void test_filesystem_base_deserialize() {

	UuidGeneratorDefault gen;
	string udn = gen.generate();
	
	FileStream fs("device.xml", "wb");
	fs.write(dd_fs(udn).c_str(), dd_fs(udn).size());
	fs.close();

	fs = FileStream("scpd.xml", "wb");
	fs.write(scpd().c_str(), scpd().size());
	fs.close();

	AutoRef<UPnPDevice> device = UPnPDeviceDeserializer::buildDevice(Url("file://" + File::getCwd() + "/device.xml"));

	ASSERT(device->getUdn(), ==, "uuid:" + udn);
	ASSERT(device->getFriendlyName(), ==, "UPnP Test Device");

	ASSERT(device->hasService("urn:schemas-dummy-com:service:Dummy:1"), ==, true);
	ASSERT(device->getService("urn:schemas-dummy-com:service:Dummy:1")->getAction("GetProtocolInfo").arguments().size(), ==, 2);
}

static void test_serialize() {
	UuidGeneratorDefault gen;
	string udn = gen.generate();
	HttpServerConfig config(9999);
	AnotherHttpServer server(config);
	server.registerRequestHandler("/*", AutoRef<HttpRequestHandler>(new RequestHandler(udn)));
	server.startAsync();

	AutoRef<UPnPDevice> device = UPnPDeviceDeserializer::buildDevice(Url("http://localhost:9999/device.xml"));

	ASSERT(device->getUdn(), ==, "uuid:" + udn);
	ASSERT(device->getFriendlyName(), ==, "UPnP Test Device");

	ASSERT(device->hasService("urn:schemas-dummy-com:service:Dummy:1"), ==, true);
	ASSERT(device->getService("urn:schemas-dummy-com:service:Dummy:1")->getAction("GetProtocolInfo").arguments().size(), ==, 2);
	
	string dd = UPnPDeviceSerializer::serializeDeviceDescription(*device);

	cout << "serialized : " << dd << endl;

	AutoRef<UPnPDevice> deserialized(new UPnPDevice);
	deserialized->baseUrl() = Url("http://localhost:9999/device.xml");
	UPnPDeviceDeserializer::parseDeviceXml(dd, *deserialized);

	ASSERT(deserialized->getUdn(), ==, "uuid:" + udn);

	server.stop();
}

int main(int argc, char *args[]) {

	cout << " ** test_deserialize" << endl;
	test_deserialize();
	cout << " ** test_filesystem_base_deserialize" << endl;
	test_filesystem_base_deserialize();
	cout << " ** test_serialize" << endl;
	test_serialize();
    
    return 0;
}

static string dd(string udn) {
		
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
		"<controlURL>/control?udn=" + udn + "&amp;serviceType=" + dummy + "</controlURL>"
		"<eventSubURL>/event?udn=" + udn + "&amp;serviceType=" + dummy + "</eventSubURL>"
		"<SCPDURL>/scpd.xml?udn=" + udn + "&amp;serviceType=" + dummy + "</SCPDURL>"
		"</service></serviceList>"
		"</root>";

	return xml;
}

static string dd_fs(string udn) {
		
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
		"<controlURL>control.xml</controlURL>"
		"<eventSubURL>event.xml</eventSubURL>"
		"<SCPDURL>scpd.xml</SCPDURL>"
		"</service></serviceList>"
		"</root>";

	return xml;
}

static string scpd() {
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
