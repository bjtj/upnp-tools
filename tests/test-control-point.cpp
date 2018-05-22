#include "utils.hpp"
#include <liboslayer/Logger.hpp>
#include <liboslayer/Uuid.hpp>
#include <libupnp-tools/SSDPHeader.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <libhttp-server/StringDataSink.hpp>

using namespace std;
using namespace osl;
using namespace http;
using namespace upnp;
using namespace ssdp;

class RequestHandler : public HttpRequestHandler {
private:
	UuidGeneratorVersion1 gen;
	string _udn;
	string dummy;
	
public:
    RequestHandler() {
		genUdn();
		dummy = "urn:schemas-dummy-com:service:Dummy:1";
	}
    virtual ~RequestHandler() {}
	
	void genUdn() {
		_udn = "uuid:" + gen.generate();
	}
	
	virtual AutoRef<DataSink> getDataSink() {
		return AutoRef<DataSink>(new StringDataSink);
	}
	
	virtual void onHttpRequestContentCompleted(HttpRequest & request, AutoRef<DataSink> sink, HttpResponse & response) {
		cout << " ** path : " << request.getRawPath() << endl;
		if (request.getPath() == "/device.xml") {
			response.setStatus(200);
			response.setContentType("text/xml");
			response.setFixedTransfer(dd(_udn));
		} else if (request.getRawPath() == "/scpd.xml?udn=" + _udn + "&serviceType=" + dummy) {
			response.setStatus(200);
			response.setContentType("text/xml");
			response.setFixedTransfer(scpd());
		} else {
			response.setStatus(404);
		}
	}
	
	string & udn() {
		return _udn;
	}
	
	SSDPHeader getSSDPHeader() {
		InetAddress addr;
		SSDPHeader header("NOTIFY * HTTP/1.1\r\n"
						  "HOST: 239.255.255.250:1900\r\n"
						  "Location: http://127.0.0.1:9003/device.xml\r\n"
						  "NTS: ssdp:alive\r\n"
						  "USN: " + _udn + "::rootdevice\r\n"
						  "\r\n", addr);
		return header;
	}
	
	string dd(const string & udn) {
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
			"<UDN>" + udn + "</UDN>"
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
};

// 
static map< string, AutoRef<UPnPDevice> > s_device_list;

// 
class DeviceListener : public UPnPDeviceListener {
private:
public:
    DeviceListener() {}
    virtual ~DeviceListener() {}
	virtual void onDeviceAdded(AutoRef<UPnPDevice> device) {
		cout << " ** added : " << device->udn() << endl;
		s_device_list[device->udn()] = device;
	}
	virtual void onDeviceRemoved(AutoRef<UPnPDevice> device) {
		cout << " ** removed : " << device->udn() << endl;
		s_device_list.erase(device->udn());
	}
};


static void test_control_point() {

	HttpServerConfig httpConfig(9003);
	AnotherHttpServer server(httpConfig);
	AutoRef<HttpRequestHandler> handler(new RequestHandler);
	server.registerRequestHandler("/*", handler);
	server.startAsync();

	UPnPControlPoint cp(UPnPControlPoint::Config(9999));
	cp.setDeviceListener(AutoRef<UPnPDeviceListener>(new DeviceListener));
	cp.startAsync();

	idle(200);

	// first device
	cp.addDevice(((RequestHandler*)&handler)->getSSDPHeader());
	idle(200);
	string udn = ((RequestHandler*)&handler)->udn();
	cout << " ** test udn : " << udn << endl;
	ASSERT(s_device_list.size(), ==, 1);
	ASSERT(s_device_list[udn]->udn(), ==, udn);

	// second device
	((RequestHandler*)&handler)->genUdn();
	cp.addDevice(((RequestHandler*)&handler)->getSSDPHeader());
	idle(100);
	udn = ((RequestHandler*)&handler)->udn();
	ASSERT(s_device_list.size(), ==, 2);
	ASSERT(s_device_list[udn]->udn(), ==, udn);

	cp.stop();
	server.stop();
}

int main(int argc, char *args[]) {

	LoggerFactory::instance().setProfile("*", "basic", "console");

	test_control_point();
    
    return 0;
}
