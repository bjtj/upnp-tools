#include <iostream>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <libupnp-tools/Uuid.hpp>
#include <libupnp-tools/UPnPModels.hpp>
#include <libupnp-tools/SSDPMsearchSender.hpp>
#include <libupnp-tools/NetworkUtil.hpp>

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

class UPnPHttpRequestHandler : public HttpRequestHandler {
private:
	string udn;
public:
    UPnPHttpRequestHandler(const string & udn) : udn(udn) {}
    virtual ~UPnPHttpRequestHandler() {}

	virtual void onHttpRequestHeaderCompleted(HttpRequest & request, HttpResponse & response) {
		string path = request.getPath();
		if (path == "/" || path == "/device.xml") {
			response.setStatusCode(200);
			response.setContentType("text/xml");
			response.getHeader().setConnection("close");
			
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
			setFixedTransfer(response, xml);
		} else if (path.find("/scpd.xml") != string::npos) {
			response.setStatusCode(200);
			response.setContentType("text/xml");
			response.getHeader().setConnection("close");
			
			string udn = request.getParameter("udn");
			string serviceType = request.getParameter("serviceType");
			string scpd = "<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">"
				"<specVersion>"
				"<major>1</major>"
				"<minor>0</minor>"
				"</specVersion>"
				"<actionList />"
				"<serviceStateTable />"
				"</scpd>";
			setFixedTransfer(response, scpd);
		} else {
			response.setStatusCode(404);
			response.setContentType("text/plain");
			response.getHeader().setConnection("close");
			
			setFixedTransfer(response, "Not found...");
		}
	}
};

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

	HttpServerConfig config;
	config.setProperty("listen.port", 9000);
	config.setProperty("thread.count", 5);
	AnotherHttpServer server(config);

	UPnPHttpRequestHandler handler(udn);
	server.registerRequestHandler("/*", &handler);

	server.startAsync();

	while (1) {
		char buffer[1024] = {0,};
		if (readline(buffer, sizeof(buffer)) > 0) {
			if (!strcmp(buffer, "q")) {
				break;
			} else if (!strcmp(buffer, "alive")) {
				OS::InetAddress addr = NetworkUtil::selectDefaultAddress();
				notifyAlive(udn, "upnp:rootdevice", "http://" + addr.getHost() + ":9000/device.xml");
			} else if (!strcmp(buffer, "byebye")) {
				notifyByeBye(udn, "upnp:rootdevice");
			}
		}
	}
    
    return 0;
}
