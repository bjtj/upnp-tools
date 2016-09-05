#include <iostream>
#include <liboslayer/XmlParser.hpp>
#include <liboslayer/MessageQueue.hpp>
#include <liboslayer/Uuid.hpp>
#include <libhttp-server/StringDataSink.hpp>
#include "UPnPServer.hpp"
#include "SSDPMsearchSender.hpp"
#include "NetworkUtil.hpp"
#include "UPnPActionRequest.hpp"
#include "UPnPActionResponse.hpp"
#include "XmlUtils.hpp"

namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace SSDP;
	using namespace HTTP;
	using namespace UTIL;

	/**
	 * @breif life time task
	 */
	class UPnPServerLifetimeTask : public TimerTask {
	private:
		UPnPServer & server;
	public:
		UPnPServerLifetimeTask(UPnPServer & server) : server(server) {}
		virtual ~UPnPServerLifetimeTask() {}
		virtual void doTask() {
			
			// TODO: notify alive profiles
			
			// TODO: check outdated resources
			server.collectOutdated();
		}
	};

	class UPnPServerSsdpHandler : public SSDPEventListener {
	private:
		UPnPServer & server;
	public:
		UPnPServerSsdpHandler(UPnPServer & server) : server(server) {}
		virtual ~UPnPServerSsdpHandler() {}
		virtual bool filter(SSDPHeader & header) {
			return true;
		}
		/**
		   M-SEARCH * HTTP/1.1
		   HOST: 239.255.255.250:1900
		   MAN: "ssdp:discover"
		   MX: 3
		   ST: upnp:rootdevice
		   USER-AGENT: Android/23 UPnP/1.1 UPnPTool/1.4.7

		   M-SEARCH * HTTP/1.1
		   MX: 1
		   ST: upnp:rootdevice
		   MAN: "ssdp:discover"
		   User-Agent: UPnP/1.0 DLNADOC/1.50 Platinum/1.0.4.11
		   Host: 239.255.255.250:1900
		   Connection: close
		*/
		virtual void onMsearch(SSDPHeader & header) {
			InetAddress remoteAddr = header.getRemoteAddr();
			server.respondMsearch(header.getSt(), remoteAddr);
		}
		virtual void onNotify(SSDPHeader & header) {
			// ignore
		}
		virtual void onMsearchResponse(SSDPHeader & header) {
			// ignore
		}
	};


	/**
	 * @brief upnp server http request handler
	 */
	class UPnPServerHttpRequestHandler : public HttpRequestHandler {
	private:
		UPnPServer & server;
	public:
		UPnPServerHttpRequestHandler(UPnPServer & server) : server(server) {}
		virtual ~UPnPServerHttpRequestHandler() {}

		virtual AutoRef<DataSink> getDataSink() {
			return AutoRef<DataSink>(new StringDataSink);
		}

		virtual void onHttpResponseTransferCompleted(HttpRequest & request, HttpResponse & response) {
			// after work -- connection not closed yet
		}

		virtual void onHttpRequestContentCompleted(HttpRequest & request, AutoRef<DataSink> sink, HttpResponse & response) {
			
			string uri = request.getHeader().getPart2();

			if (request.getPath() == "/device.xml") {

				AutoRef<UPnPDeviceProfileSession> session = server.getProfileManager().getDeviceProfileSessionWithUuid(request.getParameter("udn"));
				if (!session->isEnabled()) {
					response.setStatusCode(404);
					return;
				}
				
				response.setStatusCode(200);
				response.setContentType("text/xml");
				// UPnPDeviceProfile profile = server.getDeviceProfileWithUuid(request.getParameter("udn")); // TODO: consider udn
				UPnPDeviceProfile profile = server.getProfileManager().getDeviceProfileSessionWithUuid(request.getParameter("udn"))->profile();

				setFixedTransfer(response, profile.deviceDescription());
				return;
			}
			
			// if (server.hasDeviceProfileWithScpdUrl(uri)) {
			if (server.getProfileManager().hasDeviceProfileSessionWithScpdUrl(uri)) {
				// UPnPDeviceProfile deviceProfile = server.getDeviceProfileHasScpdUrl(uri);
				UPnPDeviceProfile deviceProfile = server.getProfileManager().getDeviceProfileSessionHasScpdUrl(uri)->profile();
				response.setStatusCode(200);
				response.setContentType("text/xml");
				UPnPServiceProfile serviceProfile = deviceProfile.getServiceProfileWithScpdUrl(uri);
				setFixedTransfer(response, serviceProfile.scpd());
				return;
			}
			
			// if (server.hasDeviceProfileWithControlUrl(uri)) {
			if (server.getProfileManager().hasDeviceProfileSessionWithControlUrl(uri)) {
				
				// TODO: recognize specific device and service
				
				UPnPActionRequest actionRequest = parseActionRequest(request);
				UPnPActionResponse actionResponse;
				actionResponse.actionName() = actionRequest.actionName();
				actionResponse.serviceType() = actionRequest.serviceType();
				handleActionRequest(actionRequest, actionResponse);

				response.setStatusCode(200);
				response.setContentType("text/xml");
				setFixedTransfer(response, makeSoapResponseContent(actionResponse));
				return;
			}

			// if (server.hasDeviceProfileWithEventSubUrl(uri)) {
			if (server.getProfileManager().hasDeviceProfileSessionWithEventSubUrl(uri)) {
				
				// event sub/unsub

				// UPnPDeviceProfile deviceProfile = server.getDeviceProfileHasEventSubUrl(uri);
				UPnPDeviceProfile deviceProfile = server.getProfileManager().getDeviceProfileSessionHasEventSubUrl(uri)->profile();
				UPnPServiceProfile serviceProfile = deviceProfile.getServiceProfileWithEventSubUrl(uri);
				UPnPPropertyManager & nc = server.getPropertyManager();

				if (request.getMethod() == "SUBSCRIBE") {
					string callbackUrls = request.getHeaderFieldIgnoreCase("CALLBACK");
					vector<string> urls = parseCallbackUrls(callbackUrls);
					string timeout = request.getParameter("TIMEOUT");
					unsigned long timeoutMilli = parseTimeout(timeout);

					UuidGeneratorVersion1 gen;
					string sid = gen.generate();

					UPnPEventSubscriptionSession session;
					session.sid() = sid;
					session.callbackUrls() = urls;
					session.setTimeout(timeoutMilli);
					session.udn() = deviceProfile.uuid(); // TODO: consider add real meaning of udn function
					session.serviceType() = serviceProfile.serviceType();

					nc.addSubscriptionSession(session);

					server.delayNotifyEvent(sid, 100);

					response.setStatusCode(200);
					response.setContentType("text/xml");
					response.getHeader().setHeaderField("SID", sid);
					return;
					
				} else if (request.getMethod() == "UNSUBSCRIBE") {
					string sid = request.getParameter("SID");
					nc.removeSubscriptionSession(sid);
					response.setStatusCode(200);
					return;
				}
			}

			response.setStatusCode(404);
			response.setContentType("text/plain");
			setFixedTransfer(response, "Not found");
		}

		void scheduleNotifyEvent(const string & sid) {
			// TODO: add to queue
		}

		void handleActionRequest(UPnPActionRequest & request, UPnPActionResponse & response) {
			if (!server.getActionRequestHandler().nil()) {
				server.getActionRequestHandler()->handleActionRequest(request, response);
			}
		}

		UPnPActionRequest parseActionRequest(HttpRequest & request) {
			UPnPActionRequest actionRequest;

			string soapAction = request.getHeaderField("SOAPACTION");
			soapAction = unwrapQuotes(soapAction);
			size_t f = soapAction.find("#");
			if (f == string::npos) {
				throw OS::Exception("wrong soap action header format", -1, 0);
			}
			string serviceType = soapAction.substr(0, f);
			string actionName = soapAction.substr(f + 1);

			actionRequest.serviceType() = serviceType;
			actionRequest.actionName() = actionName;

			string xml = ((StringDataSink*)&request.getTransfer()->sink())->data();
			XML::XmlDocument doc = XML::DomParser::parse(xml);
			if (doc.getRootNode().nil()) {
				throw OS::Exception("wrong soap action xml format", -1, 0);
			}

			XML::XmlNode * actionNode = doc.getRootNode()->getElementByTagName(actionName);
			if (actionNode == NULL) {
				throw OS::Exception("wrong soap action xml format / no action name tag", -1, 0);
			}

			vector<XML::XmlNode*> children = actionNode->children();
			for (vector<XML::XmlNode*>::iterator iter = children.begin(); iter != children.end(); iter++) {
				if (XmlUtils::testNameValueXmlNode(*iter)) {
					UTIL::NameValue nv = XmlUtils::toNameValue(*iter);
					actionRequest[nv.name()] = nv.value();
				}
			}

			return actionRequest;
		}

		string makeSoapResponseContent(UPnPActionResponse & response) {

			string actionName = response.actionName();
			string serviceType = response.serviceType();
			map<string, string> & parameters = response.parameters();
			
			string xml;
			xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n";
			xml.append("<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" "
					   "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n");
			xml.append("<s:Body>\r\n");
			xml.append("<u:" + actionName + "Response xmlns:u=\"" + serviceType + "\">\r\n");
			for (map<string, string>::iterator iter = parameters.begin(); iter != parameters.end(); iter++) {
				string name = XML::XmlEncoder::encode(iter->first);
				string value = XML::XmlEncoder::encode(iter->second);
				xml.append("<" + name + ">" + value + "</" + name + ">\r\n");
			}
			xml.append("</u:" + actionName + "Response>");
			xml.append("</s:Body>\r\n");
			xml.append("</s:Envelope>");
			return xml;
		}

		vector<string> parseCallbackUrls(const string & urls) {
			vector<string> ret;
			string buffer;
			for (string::const_iterator iter = urls.begin(); iter != urls.end(); iter++) {
				if (*iter == '<') {
					buffer = "";
					for (iter++; iter != urls.end() && *iter != '>'; iter++) {
						buffer.append(1, *iter);
					}
					ret.push_back(buffer);
				}
			}
			return ret;
		}

		unsigned long parseTimeout(const string & phrase) {
			size_t p = phrase.find("Second-"); // TODO: case insensitive
			if (p == string::npos) {
				return 0; // parsing failed
			}
			return Text::toLong(phrase.substr(p + 7));
		}

		string unwrapQuotes(const string & text) {
			string ret = text;
			if (*ret.begin() == '\"') {
				ret = ret.substr(1);
			}
			if (*ret.rbegin() == '\"') {
				ret = ret.substr(0, ret.length() - 1);
			}
			return ret;
		}
	};

	/**
	 *
	 */

	string UPnPServer::SERVER_INFO = "Net-OS 5.xx UPnP/1.0";
	
	UPnPServer::UPnPServer(const UPnPServer::Config & config)
		: config(config),
		  httpServer(NULL),
		  notificationThread(propertyManager),
		  ssdpListener(new UPnPServerSsdpHandler(*this)) {
	}

	UPnPServer::UPnPServer(const UPnPServer::Config & config, AutoRef<NetworkStateManager> networkStateManager)
		: networkStateManager(networkStateManager),
		  config(config),
		  httpServer(NULL),
		  notificationThread(propertyManager),
		  ssdpListener(new UPnPServerSsdpHandler(*this)) {
	}
	
	UPnPServer::~UPnPServer() {
	}

	void UPnPServer::startAsync() {
		
		if (httpServer) {
			return;
		}
		
		HttpServerConfig httpServerConfig;
		httpServerConfig["listen.port"] = config["listen.port"];
		httpServerConfig["thread.count"] = config.getProperty("thread.count", "5");
		httpServer = new AnotherHttpServer(httpServerConfig);
		AutoRef<HttpRequestHandler> handler(new UPnPServerHttpRequestHandler(*this));
		httpServer->registerRequestHandler("/*", handler);
		httpServer->startAsync();

		notificationThread.start();
		
		timerThread.start();
		timerThread.looper().interval(10 * 1000, AutoRef<TimerTask>(new UPnPServerLifetimeTask(*this)));

		ssdpServer.addSSDPEventListener(ssdpListener);
		ssdpServer.startAsync();
	}
	
	void UPnPServer::stop() {
		
		if (!httpServer) {
			return;
		}

		ssdpServer.stop();

		timerThread.stop();
		timerThread.join();
		
		notificationThread.interrupt();
		notificationThread.join();
		
		httpServer->stop();
		delete httpServer;
		httpServer = NULL;
	}

	string UPnPServer::makeLocation(UPnPDeviceProfile & deviceProfile) {
		Url url;
		url.setProtocol("http");
		url.setHost(NetworkUtil::selectDefaultAddress().getHost());
		url.setPort(config["listen.port"]);
		url.setPath("device.xml");
		url.setParameter("udn", deviceProfile.uuid()); // TODO: consider what is better between udn and uuid
		return url.toString();
	}
	void UPnPServer::notifyAlive(UPnPDeviceProfile & profile) {

		SSDPMsearchSender sender;
		string host = "239.255.255.250";
		int port = 1900;

		string uuid = profile.uuid();
		string location = makeLocation(profile);
		sender.sendMcastToAllInterfaces(makeNotifyAlive(location, uuid, "upnp:rootdevice"), host, port);

		vector<string> & deviceTypes = profile.deviceTypes();
		for (vector<string>::iterator iter = deviceTypes.begin(); iter != deviceTypes.end(); iter++) {
			sender.sendMcastToAllInterfaces(makeNotifyAlive(location, uuid, *iter), host, port);
		}

		vector<UPnPServiceProfile> & serviceProfiles = profile.serviceProfiles();
		for (vector<UPnPServiceProfile>::iterator iter = serviceProfiles.begin(); iter != serviceProfiles.end(); iter++) {
			sender.sendMcastToAllInterfaces(makeNotifyAlive(location, uuid, iter->serviceType()), host, port);
		}
		
		sender.close();
	}
	void UPnPServer::notifyAliveWithDeviceType(UPnPDeviceProfile & profile, const string & deviceType) {

		string uuid = profile.uuid();
		string location = makeLocation(profile);
		
		SSDPMsearchSender sender;
		sender.sendMcastToAllInterfaces(makeNotifyAlive(location, uuid, deviceType), "239.255.255.250", 1900);
		sender.close();
	}
	string UPnPServer::makeNotifyAlive(const string & location, const string & uuid, const string & deviceType) {
		return "NOTIFY * HTTP/1.1\r\n"
			"Cache-Control: max-age=1800\r\n"
			"HOST: 239.255.255.250:1900\r\n"
			"Location: " + location + "\r\n"
			"NT: " + deviceType + "\r\n"
			"NTS: ssdp:alive\r\n"
			"Server: " + SERVER_INFO + "\r\n"
			"USN: uuid:" + uuid + "::" + deviceType + "\r\n"
			"\r\n";
	}
	void UPnPServer::notifyByeBye(UPnPDeviceProfile & profile) {

		SSDPMsearchSender sender;
		string host = "239.255.255.250";
		int port = 1900;

		string uuid = profile.uuid();
		string location = makeLocation(profile);
		sender.sendMcastToAllInterfaces(makeNotifyByeBye(uuid, "upnp:rootdevice"), host, port);

		vector<string> & deviceTypes = profile.deviceTypes();
		for (vector<string>::iterator iter = deviceTypes.begin(); iter != deviceTypes.end(); iter++) {
			sender.sendMcastToAllInterfaces(makeNotifyByeBye(uuid, *iter), host, port);
		}

		vector<UPnPServiceProfile> & serviceProfiles = profile.serviceProfiles();
		for (vector<UPnPServiceProfile>::iterator iter = serviceProfiles.begin(); iter != serviceProfiles.end(); iter++) {
			sender.sendMcastToAllInterfaces(makeNotifyByeBye(uuid, iter->serviceType()), host, port);
		}
		
		sender.close();
	}
	void UPnPServer::notifyByeByeWithDeviceType(UPnPDeviceProfile & profile, const string & deviceType) {

		string uuid = profile.uuid();
		
		SSDPMsearchSender sender;
		sender.sendMcastToAllInterfaces(makeNotifyByeBye(uuid, deviceType), "239.255.255.250", 1900);
		sender.close();
	}
	string UPnPServer::makeNotifyByeBye(const string & uuid, const string & deviceType) {
		return "NOTIFY * HTTP/1.1\r\n"
			"Host: 239.255.255.250:1900\r\n"
			"NT: " + (deviceType.empty() ? "uuid:" + uuid : deviceType)  + "\r\n"
			"NTS: ssdp:byebye\r\n"
			"USN: uuid:" + uuid + (deviceType.empty() ? "" : "::" + deviceType) + "\r\n"
			"\r\n";
	}

	void UPnPServer::respondMsearch(const string & st, InetAddress & remoteAddr) {

		SSDPMsearchSender sender;
		
		// vector<UPnPDeviceProfile> profiles = searchProfiles(st);
		vector<AutoRef<UPnPDeviceProfileSession> > profiles = getProfileManager().searchProfileSessions(st);
		for (size_t i = 0; i < profiles.size(); i++) {
			string uuid = profiles[i]->profile().uuid();
			string location = makeLocation(profiles[i]->profile());
			sender.unicast(makeMsearchResponse(location, uuid, st), remoteAddr);
		}

		sender.close();
	}
	string UPnPServer::makeMsearchResponse(const string & location, const string & uuid, const string & st) {
		return "HTTP/1.1 200 OK\r\n"
			"Cache-Control: max-age=1800\r\n"
			"HOST: 239.255.255.250:1900\r\n"
			"Location: " + location + "\r\n"
			"ST: " + st + "\r\n"
			"Server: " + SERVER_INFO + "\r\n"
			"USN: uuid:" + uuid + "::" + st + "\r\n"
			"Ext: \r\n"
			"\r\n";
	}

	UPnPDeviceProfileSessionManager & UPnPServer::getProfileManager() {
		return profileManager;
	}
	
	void UPnPServer::registerDeviceProfile(const string & uuid, const UPnPDeviceProfile & profile) {
		getProfileManager().registerProfile(uuid, profile);
	}

	void UPnPServer::unregisterDeviceProfile(const std::string & uuid) {
		getProfileManager().unregisterProfile(uuid);
	}
	
	void UPnPServer::setActionRequestHandler(AutoRef<UPnPActionRequestHandler> actionRequestHandler) {
		this->actionRequestHandler = actionRequestHandler;
	}

	AutoRef<UPnPActionRequestHandler> UPnPServer::getActionRequestHandler() {
		return actionRequestHandler;
	}

	UPnPPropertyManager & UPnPServer::getPropertyManager() {
		return propertyManager;
	}

	void UPnPServer::notifyEvent(const string & sid) {
		notificationThread.notify(sid);
	}
	
	void UPnPServer::delayNotifyEvent(const string & sid, unsigned long delay) {
		notificationThread.delayNotify(sid, delay);
	}

	TimerLooperThread & UPnPServer::getTimerThread() {
		return timerThread;
	}

	void UPnPServer::collectOutdated() {
		propertyManager.collectOutdated();
	}
}
