#include "utils.hpp"
#include <liboslayer/Uuid.hpp>
#include <libupnp-tools/SSDPHeader.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <libhttp-server/StringDataSink.hpp>
#include <libhttp-server/WebServerUtil.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace HTTP;
using namespace UPNP;
using namespace SSDP;

class RequestHandler : public HttpRequestHandler, public WebServerUtil {
private:
	string _udn;
	string dummy;
public:
    RequestHandler() {
		UuidGeneratorVersion1 gen;
		_udn = gen.generate();
		dummy = "urn:schemas-dummy-com:service:Dummy:1";
	}
    virtual ~RequestHandler() {}
	virtual AutoRef<DataSink> getDataSink() {
		return AutoRef<DataSink>(new StringDataSink);
	}
	virtual void onHttpRequestContentCompleted(HttpRequest & request, AutoRef<DataSink> sink, HttpResponse & response) {
		cout << " ** path : " << request.getRawPath() << endl;
		if (request.getPath() == "/device.xml") {
			response.setStatus(200);
			response.setContentType("text/xml");
			setFixedTransfer(response, dd(_udn));
		} else if (request.getRawPath() == "/scpd.xml?udn=" + _udn + "&serviceType=" + dummy) {
			response.setStatus(200);
			response.setContentType("text/xml");
			setFixedTransfer(response, scpd());
		} else {
			response.setStatus(404);
		}
	}
	string udn() {
		return _udn;
	}
	SSDPHeader getSSDPHeader() {
		OS::InetAddress addr;
		SSDPHeader header("NOTIFY * HTTP/1.1\r\n"
						  "HOST: 239.255.255.250:1900\r\n"
						  "Location: http://127.0.0.1:9998/device.xml\r\n"
						  "NTS: ssdp:alive\r\n"
						  "USN: uuid:" + _udn + "::rootdevice\r\n"
						  "\r\n", addr);
		return header;
	}
	string dd(string udn) {
		
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

static map<string, AutoRef<UPnPDevice> > s_device_list;

class DeviceListener : public DeviceAddRemoveListener {
private:
public:
    DeviceListener() {}
    virtual ~DeviceListener() {}
	virtual void onDeviceAdd(AutoRef<UPnPDevice> device) {
		Uuid uuid(device->getUdn());
		cout << " ** added : " << uuid.getUuid() << endl;
		s_device_list[uuid.getUuid()] = device;
	}
	virtual void onDeviceRemove(AutoRef<UPnPDevice> device) {
		Uuid uuid(device->getUdn());
		cout << " ** removed : " << uuid.getUuid() << endl;
		s_device_list.erase(uuid.getUuid());
	}
};


static void test_control_point() {

	HttpServerConfig httpConfig(9998);
	AnotherHttpServer server(httpConfig);
	AutoRef<HttpRequestHandler> handler(new RequestHandler);
	server.registerRequestHandler("/*", handler);
	server.startAsync();

	UPnPControlPoint cp(UPnPControlPoint::Config(9999));
	cp.setDeviceAddRemoveListener(AutoRef<DeviceAddRemoveListener>(new DeviceListener));
	cp.startAsync();

	SSDPHeader header = ((RequestHandler*)&handler)->getSSDPHeader();
	cp.addDevice(header);

	idle(100);

	string udn = ((RequestHandler*)&handler)->udn();
	cout << " ** test udn : " << udn << endl;
	ASSERT(s_device_list.size(), ==, 1);
	ASSERT(s_device_list[udn]->getUdn(), ==, "uuid:" + udn);

	cp.stop();
	server.stop();
}

int main(int argc, char *args[]) {

	test_control_point();
    
    return 0;
}
