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

static void s_set_dial_devcie(UPnPServer & server, const string & dd_path, const string & udn) {
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
		response.setHeaderField("Application-URL", "http://" + host + ":9001/dial/");
	}
	virtual void onScpdRequest(http::HttpRequest & request, http::HttpResponse & response) {
	}
	virtual void onControlRequest(http::HttpRequest & request, http::HttpResponse & response) {
	}
};

class DialApp
{
private:
	string _name;
	bool _running;
public:
    DialApp(const string & name) : _name(name), _running(false) {
	}
    virtual ~DialApp() {
	}
	string & name() {
		return _name;
	}
	bool running() {
		return _running;
	}
	void launch() {
		_running = true;
	}
	void stop() {
		_running = false;
	}
};

/**
 * 
 */
class DialRequestHandler : public HttpRequestHandler
{
private:
	vector<DialApp> _apps;
public:
    DialRequestHandler() {
		_apps.push_back(DialApp("YouTube"));
	}
    virtual ~DialRequestHandler() {}
	virtual void onHttpRequestContentCompleted(HttpRequest & request, AutoRef<DataSink> sink, HttpResponse & response) {
		DialApp & app = _apps[0];
		if (request.getMethod() == "GET") {
			cout << "[GET] status" << endl;
			response.setStatus(200);
			string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
			xml.append("<service xmlns=\"urn:dial-multiscreen-org:schemas:dial\">\n");
			xml.append("    <name>" + app.name() + "</name>\n");
			xml.append("    <options allowStop=\"true\" />\n");
			xml.append("    <state>" + string(app.running() ? "running" : "stopped") + "</state>\n");
			xml.append("</service>");
			response.setFixedTransfer(xml);
		} else if (request.getMethod() == "POST") {
			cout << "[POST] launch" << endl;
			app.launch();
			response.setStatus(201);
		} else if (request.getMethod() == "DELETE") {
			cout << "[DELETE] stop" << endl;
			app.stop();
			response.setStatus(200);
		} else {
			cout << "no operation" << endl;
		}
	}
};


/**
 * 
 */
int main(int argc, char *argv[])
{

#if defined(_DEBUG)
	LoggerFactory::instance().setProfile("*", "basic", "console");
#endif
	
	string ddPath = "dial.xml";
	if (argc > 1) {
		ddPath = string(argv[1]);
	}
	
	UuidGeneratorVersion1 gen;
	string uuid = gen.generate();
	string udn = uuid;

	UPnPServer::Config config(9001);
	config["thread.count"] = "50";
    UPnPServer server(config);
	s_set_dial_devcie(server, ddPath, udn);
	server.setHttpEventListener(AutoRef<HttpEventListener>(new DialHttpEventListener()));
	server.startAsync();
	server.getHttpServer()->registerRequestHandler("/dial*", AutoRef<HttpRequestHandler>(new DialRequestHandler));
	server.activateDevice(udn);
	cout << "(press any key to stop)" << endl;
	getchar();
	server.deactivateDevice(udn);
	server.stop();
	cout << "BYE." << endl;
    return 0;
}
