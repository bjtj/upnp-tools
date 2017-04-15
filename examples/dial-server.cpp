#include <iostream>
#include <liboslayer/Uuid.hpp>
#include <liboslayer/FileStream.hpp>
#include <libupnp-tools/UPnPServer.hpp>
#include <libupnp-tools/UPnPActionRequestHandler.hpp>
#include <libupnp-tools/UPnPDeviceDeserializer.hpp>
#include <libupnp-tools/UPnPResourceManager.hpp>
#include <libupnp-tools/UPnPDeviceProfileBuilder.hpp>
#include <libupnp-tools/UPnPResourceManager.hpp>
#include <libupnp-tools/NetworkUtil.hpp>
#include <libhttp-server/WebServerUtil.hpp>
#include <libhttp-server/Url.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace HTTP;
using namespace UPNP;

static void s_set_dial_devcie(UPnPServer & server, const string & dd_path, const string & uuid) {
	FileStream fs(dd_path, "r");
	string xml = fs.readFullAsString();
	UPnPResourceManager::properties()["/device.xml"] = xml;
	server.registerDeviceProfile(uuid, Url("prop:///device.xml"));
}

/**
 * 
 */
class DialHttpEventListener : public HttpEventListener {
public:
    DialHttpEventListener() {
	}
	
    virtual ~DialHttpEventListener() {
	}
	
	virtual void onDeviceDescriptionRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response) {
		string host = NetworkUtil::selectDefaultAddress().getHost();
		response.setHeaderField("Application-URL",
										 "http://" + host + ":9001/dial/");
	}
	virtual void onScpdRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response) {
	}
	virtual void onControlRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response) {
	}
};

/**
 * 
 */
class DialRequestHandler : public HttpRequestHandler, public WebServerUtil
{
public:
    DialRequestHandler() {}
    virtual ~DialRequestHandler() {}
	virtual void onHttpRequestContentCompleted(HttpRequest & request, AutoRef<DataSink> sink, HttpResponse & response) {
		if (request.getMethod() == "GET") {
			response.setStatus(200);
			string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
			xml.append("<service xmlns=\"urn:dial-multiscreen-org:schemas:dial\">\n");
			xml.append("    <name>YouTube</name>\n");
			xml.append("    <options allowStop=\"true\" />\n");
			xml.append("    <state>stopped</state>\n");
			xml.append("</service>");
			setFixedTransfer(response, xml);
		} else if (request.getMethod() == "POST") {
			response.setStatus(201);
		} else if (request.getMethod() == "DELETE") {
			response.setStatus(200);
		}
	}
};


/**
 * 
 */
int main(int argc, char *argv[])
{
	string ddPath = "dial.xml";
	if (argc > 1) {
		ddPath = string(argv[1]);
	}
	
	UuidGeneratorVersion1 gen;
	string uuid = gen.generate();
	
    UPnPServer server(UPnPServer::Config(9001));
	s_set_dial_devcie(server, ddPath, uuid);
	server.setHttpEventListener(AutoRef<HttpEventListener>(new DialHttpEventListener()));
	server.startAsync();
	server.getHttpServer()->registerRequestHandler("/dial*",
												   AutoRef<HttpRequestHandler>(new DialRequestHandler));
	server.setEnableDevice(uuid, true);
	getchar();
	server.setEnableDevice(uuid, false);
	server.stop();
    return 0;
}
