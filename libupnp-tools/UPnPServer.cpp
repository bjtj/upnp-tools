#include "UPnPServer.hpp"
#include "SSDPMsearchSender.hpp"
#include "NetworkUtil.hpp"

namespace UPNP {

	using namespace std;
	using namespace SSDP;
	using namespace HTTP;
	using namespace UTIL;

	/**
	 * @brief upnp server http request handler
	 */
	class UPnPServerHttpRequestHandler : public HttpRequestHandler {
	private:
		string prefix;
		UPnPServer & server;
	public:
		UPnPServerHttpRequestHandler(const string & prefix, UPnPServer & server) : prefix(prefix), server(server) {}
		virtual ~UPnPServerHttpRequestHandler() {}

		virtual void onHttpRequestHeaderCompleted(HttpRequest & request, HttpResponse & response) {
			// serve dd
			// serve scpd

			// ** next plan
			// UPnPDeviceProfile profile = server.getProfileWithUdn(udn);
			// string dd = profile.getDescription();
			// string scpd = profile.getScpd(serviceType);
			// setFixedTransfer(response, dd);
			// setFixedTransfer(response, scpd);
			
			if (request.getPath() == "/device.xml") {
				response.setStatusCode(200);
				response.setContentType("text/xml");
				UPnPDeviceProfile profile = server.getDeviceProfileWithUdn(request.getParameter("udn"));
				setFixedTransfer(response, profile.deviceDescription());
				return;
			}

			string scpdUrl = request.getPath().substr(prefix.length());
			if (server.hasDeviceProfileWithScpdUrl(scpdUrl)) {
				UPnPDeviceProfile profile = server.getDeviceProfileHasScpdUrl(scpdUrl);
				response.setStatusCode(200);
				response.setContentType("text/xml");
				UPnPService service = profile.getServiceWithScpdUrl(scpdUrl);
				setFixedTransfer(response, profile.scpd(service.getServiceType()));
				return;
			}

			response.setStatusCode(404);
			response.setContentType("text/plain");
			setFixedTransfer(response, "Not found");
			
		}
		virtual void onHttpRequestContentCompleted(HttpRequest & request, HttpResponse & response) {
			// serve action invoke
			// serve event sub
			// serve event unsub

			// ** next plan
			// server.handleActionRequet();
			// server.handleEventSubscribe();
			// server.handleEventUnsubscribe();
		}
	};
	
	UPnPServer::UPnPServer(UPnPServerProfile & profile) : httpServer(NULL), profile(profile) {
	}
	UPnPServer::~UPnPServer() {
	}

	void UPnPServer::startAsync() {
		if (httpServer) {
			return;
		}
		HttpServerConfig config;
		config["listen.port"] = profile["listen.port"];
		config["thread.count"] = "5";
		httpServer = new AnotherHttpServer(config);
		AutoRef<HttpRequestHandler> handler(new UPnPServerHttpRequestHandler("/", *this));
		httpServer->registerRequestHandler("/*", handler);
		httpServer->startAsync();
	}
	void UPnPServer::stop() {
		if (!httpServer) {
			return;
		}
		httpServer->stop();
		delete httpServer;
		httpServer = NULL;
	}

	string UPnPServer::makeLocation(UPnPDeviceProfile & deviceProfile) {
		Url url;
		url.setProtocol("http");
		url.setHost(NetworkUtil::selectDefaultAddress().getHost());
		url.setPort(profile["listen.port"]);
		url.setPath("device.xml");
		url.setParameter("udn", deviceProfile.udn());
		return url.toString();
	}
		
	void UPnPServer::notifyAliveWithDeviceType(UPnPDeviceProfile & profile, const string & deviceType) {

		// location
		// device type
		// ssdp group host
		// ssdp port
		// server profile

		string udn = profile.udn();
		string location = makeLocation(profile);
		
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
	void UPnPServer::notifyByeByeWithDeviceType(UPnPDeviceProfile & profile, const string & deviceType) {

		// device type
		// ssdp group host
		// ssdp port

		string udn = profile.udn();
		
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

	bool UPnPServer::hasDeviceProfileWithScpdUrl(const std::string & scpdUrl) {
		for (map<string, UPnPDeviceProfile>::iterator iter = deviceProfiles.begin(); iter != deviceProfiles.end(); iter++) {
			if (iter->second.hasServiceWithScpdUrl(scpdUrl)) {
				return true;
			}
		}
		return false;
	}

	UPnPDeviceProfile & UPnPServer::getDeviceProfileWithUdn(const string & udn) {
		for (map<string, UPnPDeviceProfile>::iterator iter = deviceProfiles.begin(); iter != deviceProfiles.end(); iter++) {
			if (iter->second.udn() == udn) {
				return iter->second;
			}
		}
		throw OS::Exception("not found deivce profile", -1, 0);
	}

	UPnPDeviceProfile & UPnPServer::getDeviceProfileWithAlias(const string & alias) {
		for (map<string, UPnPDeviceProfile>::iterator iter = deviceProfiles.begin(); iter != deviceProfiles.end(); iter++) {
			if (iter->second.alias() == alias) {
				return iter->second;
			}
		}
		throw OS::Exception("not found deivce profile", -1, 0);
	}

	UPnPDeviceProfile & UPnPServer::getDeviceProfileHasScpdUrl(const std::string & scpdUrl) {
		for (map<string, UPnPDeviceProfile>::iterator iter = deviceProfiles.begin(); iter != deviceProfiles.end(); iter++) {
			if (iter->second.hasServiceWithScpdUrl(scpdUrl)) {
				return iter->second;
			}
		}
		throw OS::Exception("not found deivce profile", -1, 0);
	}
	
	UPnPDeviceProfile & UPnPServer::operator[] (const string & udn) {
		return deviceProfiles[udn];
	}
}
