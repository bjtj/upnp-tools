#include <iostream>
#include <liboslayer/Uuid.hpp>
#include <liboslayer/FileStream.hpp>
#include <liboslayer/Logger.hpp>
#include <libupnp-tools/UPnPServer.hpp>
#include <libupnp-tools/UPnPActionRequestHandler.hpp>
#include <libupnp-tools/UPnPDeviceDeserializer.hpp>
#include <libupnp-tools/UPnPResourceManager.hpp>
#include <libupnp-tools/NetworkUtil.hpp>
#include <libhttp-server/Url.hpp>

using namespace std;
using namespace osl;
using namespace http;
using namespace upnp;

// #define _DEBUG

static void s_set_dial_devcie(UPnPServer & server, const string & dd_path, const UDN & udn) {
	UPnPResourceManager & resMan = UPnPResourceManager::instance();
	FileStream fs(dd_path, "r");
	string xml = fs.readFullAsString();
	resMan.properties()["/device.xml"] = xml;
	AutoRef<UPnPDeviceProfile> profile = server.registerDeviceProfile(Url("prop:///device.xml"));
	profile->setUdn(udn);
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
	
	virtual void onDeviceDescriptionRequest(http::HttpRequest & request, http::HttpResponse & response) {
		string host = NetworkUtil::selectDefaultAddress().getHost();
		response.setHeaderField("Application-URL",
										 "http://" + host + ":9001/dial/");
	}
	virtual void onScpdRequest(http::HttpRequest & request, http::HttpResponse & response) {
	}
	virtual void onControlRequest(http::HttpRequest & request, http::HttpResponse & response) {
	}
};

/**
 * 
 */
class DialRequestHandler : public HttpRequestHandler
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
			response.setFixedTransfer(xml);
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

#if defined(_DEBUG)
	LoggerFactory::instance().setLoggerDescriptorSimple("*", "basic", "console");
#endif
	
	string ddPath = "dial.xml";
	if (argc > 1) {
		ddPath = string(argv[1]);
	}
	
	UuidGeneratorVersion1 gen;
	string uuid = gen.generate();
	UDN udn("uuid:" + uuid);
	
    UPnPServer server(UPnPServer::Config(9001));
	s_set_dial_devcie(server, ddPath, udn);
	server.setHttpEventListener(AutoRef<HttpEventListener>(new DialHttpEventListener()));
	server.startAsync();
	server.getHttpServer()->registerRequestHandler("/dial*",
												   AutoRef<HttpRequestHandler>(new DialRequestHandler));
	server.activateDevice(udn);
	getchar();
	server.deactivateDevice(udn);
	server.stop();
    return 0;
}
