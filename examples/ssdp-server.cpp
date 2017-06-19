#include <iostream>
#include <liboslayer/FileStream.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <libhttp-server/HttpHeader.hpp>
#include <libhttp-server/WebServerUtil.hpp>
#include <libupnp-tools/SSDPServer.hpp>
#include <libupnp-tools/NetworkUtil.hpp>

using namespace std;
using namespace OS;
using namespace HTTP;
using namespace UTIL;
using namespace SSDP;
using namespace UPNP;

static const char * dd = 
"<?xml version=\"1.0\"?>\n"
"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\n"
"<specVersion>\n"
"<major>1</major>\n"
"<minor>0</minor>\n"
"</specVersion><device>\n"
"<deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType>\n"
"<friendlyName>EFM Networks ipTIME A2004NS-R (COPY)</friendlyName>\n"
"<manufacturer>EFM Networks</manufacturer>\n"
"<manufacturerURL>www.iptime.co.kr</manufacturerURL>\n"
"<modelDescription>EFM Networks ipTIME A2004NS-R IUX</modelDescription>\n"
"<modelName>ipTIME A2004NS-R</modelName>\n"
"<modelNumber>1</modelNumber>\n"
"<modelURL>www.iptime.co.kr</modelURL>\n"
"<serialNumber>12345678</serialNumber>\n"
"<presentationURL>http://192.168.0.1/</presentationURL>\n"
"<UDN>uuid:fc4ec57e-b051-11db-88f8-0060085db3f0</UDN>\n"
"<serviceList>\n"
"</serviceList></device></root>\n";

static string getLocation() {
	return "http://" + NetworkUtil::selectDefaultAddress().getHost() + ":9000/";
}

/**
 * 
 */
class SimpleSSDPEventListener : public SSDPEventListener {
private:
public:
	SimpleSSDPEventListener() {}
	virtual ~SimpleSSDPEventListener() {}

	virtual void onMsearch(SSDPHeader & header) {
		cout << "[RECV] M-SEARCH / search target: '" << header["ST"] << "' - " << header.getRemoteAddr().toString() << endl;

		HttpHeader httpHeader;
		httpHeader.setParts("HTTP/1.1", "200", "OK"); 
		httpHeader.setHeaderField("LOCATION", getLocation());
		httpHeader.setHeaderField("USN", "uuid:fc4ec57e-b051-11db-88f8-0060085db3f0::upnp:rootdevice");
		httpHeader.setHeaderField("ST", "upnp:rootdevice");
		httpHeader.setHeaderField("CACHE-CONTROL", "max-age=1800");
		httpHeader.setHeaderField("EXT", "");

		DatagramSocket socket;
		string data = httpHeader.toString();
		DatagramPacket packet(data.size(), header.getRemoteAddr());
		packet.write(data);
		socket.send(packet);
		socket.close();
	}
};

class SimpleHttpRequestHandler : public HttpRequestHandler, public WebServerUtil {
private:
public:
	SimpleHttpRequestHandler() {
	}
	virtual ~SimpleHttpRequestHandler() {
	}
	virtual void onHttpRequestContentCompleted(HttpRequest & request, AutoRef<DataSink> sink, HttpResponse & response) {
		response.setStatus(200);
		setFixedTransfer(response, dd);
	}
};

int main(int argc, char *args[]) {

	bool done = false;
	SSDPServer server;
	AnotherHttpServer httpServer(HttpServerConfig(9000));

	httpServer.startAsync();
	httpServer.registerRequestHandler("/*", AutoRef<HttpRequestHandler>(new SimpleHttpRequestHandler));

	server.startAsync();
	server.addSSDPEventListener(AutoRef<SSDPEventListener>(new SimpleSSDPEventListener));

	while (!done) {
		FileStream fs(stdin);
		string line = fs.readline();
		if (line == "q" || line == "quit") {
			break;
		}
		if (line == "alive") {
			SSDPHeader header;
			header = server.getNotifyHeader("ssdp:alive", USN("uuid:fc4ec57e-b051-11db-88f8-0060085db3f0"));
			header.setLocation(getLocation());
			server.sendNotify(header);
			header = server.getNotifyHeader("ssdp:alive", USN("uuid:fc4ec57e-b051-11db-88f8-0060085db3f0::upnp:rootdevice"));
			header.setLocation(getLocation());
			server.sendNotify(header);
			header = server.getNotifyHeader("ssdp:alive", USN("uuid:fc4ec57e-b051-11db-88f8-0060085db3f0::urn:schemas-upnp-org:device:InternetGatewayDevice:1"));
			header.setLocation(getLocation());
			server.sendNotify(header);
		}
		if (line == "byebye") {
			SSDPHeader header;
			header = server.getNotifyHeader("ssdp:byebye", USN("uuid:fc4ec57e-b051-11db-88f8-0060085db3f0::urn:schemas-upnp-org:device:InternetGatewayDevice:1"));
			server.sendNotify(header);
			header = server.getNotifyHeader("ssdp:byebye", USN("uuid:fc4ec57e-b051-11db-88f8-0060085db3f0::upnp:rootdevice"));
			server.sendNotify(header);
			header = server.getNotifyHeader("ssdp:byebye", USN("uuid:fc4ec57e-b051-11db-88f8-0060085db3f0"));
			server.sendNotify(header);
		}
	}

	server.stop();
	httpServer.stop();
    
    return 0;
}
