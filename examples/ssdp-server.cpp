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
"<friendlyName>EFM Networks ipTIME A2004NS-R</friendlyName>\n"
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
"</serviceList></device></deviceList></device></deviceList></device></root>\n";

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
		httpHeader.setPart1("HTTP/1.1"); 
		httpHeader.setPart2("200"); 
		httpHeader.setPart3("OK");		
		httpHeader.setHeaderField("LOCATION", "http://" + NetworkUtil::selectDefaultAddress().getHost() + ":9000/");
		httpHeader.setHeaderField("USN", "uuid:fc4ec57e-b051-11db-88f8-0060085db3f0::upnp:rootdevice");
		httpHeader.setHeaderField("ST", "upnp:rootdevice");
		httpHeader.setHeaderField("CACHE-CONTROL", "max-age=120");
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
	}

	server.stop();
	httpServer.stop();
    
    return 0;
}
