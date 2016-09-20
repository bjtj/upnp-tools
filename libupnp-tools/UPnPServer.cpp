#include <liboslayer/XmlParser.hpp>
#include <liboslayer/MessageQueue.hpp>
#include <liboslayer/Uuid.hpp>
#include <libhttp-server/StringDataSink.hpp>
#include "UPnPDeviceDeserializer.hpp"
#include "UPnPDeviceProfileBuilder.hpp"
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
	 * @brief 
	 */

	UPnPDeviceProfileSession::UPnPDeviceProfileSession(const UPnPDeviceProfile & profile) : enabled(false), _profile(profile){
	}
	UPnPDeviceProfileSession::~UPnPDeviceProfileSession() {
	}
	UPnPDeviceProfile & UPnPDeviceProfileSession::profile() {
		return _profile;
	}
	void UPnPDeviceProfileSession::setEnable(bool enable) {
		this->enabled = enable;
	}
	bool UPnPDeviceProfileSession::isEnabled() {
		return enabled;
	}
	
	/**
	 * @brief 
	 */
	
	UPnPDeviceProfileSessionManager::UPnPDeviceProfileSessionManager() {
	}
	UPnPDeviceProfileSessionManager::~UPnPDeviceProfileSessionManager() {
	}
	map<string, AutoRef<UPnPDeviceProfileSession> > & UPnPDeviceProfileSessionManager::sessions() {
		return _sessions;
	}
	vector<UTIL::AutoRef<UPnPDeviceProfileSession> > UPnPDeviceProfileSessionManager::sessionList() {
		vector<AutoRef<UPnPDeviceProfileSession> > ret;
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			ret.push_back(iter->second);
		}
		return ret;
	}

	void UPnPDeviceProfileSessionManager::registerProfile(const UPnPDeviceProfile & profile) {
		registerProfile(profile.const_uuid(), profile);
	}

	void UPnPDeviceProfileSessionManager::registerProfile(const string & uuid, const UPnPDeviceProfile & profile) {
		_sessions[uuid] = AutoRef<UPnPDeviceProfileSession>(new UPnPDeviceProfileSession(profile));
	}

	void UPnPDeviceProfileSessionManager::unregisterProfile(const string & uuid) {
		_sessions.erase(uuid);
	}
	
	vector<AutoRef<UPnPDeviceProfileSession> > UPnPDeviceProfileSessionManager::searchProfileSessions(const string & st) {
		vector<AutoRef<UPnPDeviceProfileSession> > ret;
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> session = iter->second;
			if (session->profile().match(st)) {
				ret.push_back(session);
			}
		}
		return ret;
	}
	bool UPnPDeviceProfileSessionManager::hasDeviceProfileSessionByScpdUrl(const string & scpdUrl) {
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> session = iter->second;
			if (session->profile().hasServiceByScpdUrl(scpdUrl)) {
				return true;
			}
		}
		return false;
	}
	bool UPnPDeviceProfileSessionManager::hasDeviceProfileSessionByControlUrl(const string & controlUrl) {
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> session = iter->second;
			if (session->profile().hasServiceByControlUrl(controlUrl)) {
				return true;
			}
		}
		return false;
	}
	bool UPnPDeviceProfileSessionManager::hasDeviceProfileSessionByEventSubUrl(const string & eventSubUrl) {
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> session = iter->second;
			if (session->profile().hasServiceByEventSubUrl(eventSubUrl)) {
				return true;
			}
		}
		return false;
	}
	AutoRef<UPnPDeviceProfileSession> UPnPDeviceProfileSessionManager::getDeviceProfileSessionByUuid(const string & uuid) {
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> session = iter->second;
			if (session->profile().uuid() == uuid) {
				return iter->second;
			}
		}
		throw Exception("not found deivce profile session");
	}
	AutoRef<UPnPDeviceProfileSession> UPnPDeviceProfileSessionManager::getDeviceProfileSessionHasScpdUrl(const string & scpdUrl) {
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> session = iter->second;
			if (session->profile().hasServiceByScpdUrl(scpdUrl)) {
				return iter->second;
			}
		}
		throw Exception("not found deivce profile session");
	}
	AutoRef<UPnPDeviceProfileSession> UPnPDeviceProfileSessionManager::getDeviceProfileSessionHasEventSubUrl(const string & eventSubUrl) {
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> session = iter->second;
			if (session->profile().hasServiceByEventSubUrl(eventSubUrl)) {
				return iter->second;
			}
		}
		throw Exception("not found deivce profile session");
	}
	

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
			server.notifyAliveAll();
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
			server.debug("ssdp", header.toString());
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
				onDeviceDescriptionRequest(request, response, uri);
				return;
			}
			
			if (server.getProfileManager().hasDeviceProfileSessionByScpdUrl(uri)) {
				onScpdRequest(request, response, uri);
				return;
			}
			
			if (server.getProfileManager().hasDeviceProfileSessionByControlUrl(uri)) {
				server.debug("upnp:control", request.getHeader().toString());
				onControlRequest(request, response, uri);
				return;
			}

			if (server.getProfileManager().hasDeviceProfileSessionByEventSubUrl(uri)) {
				server.debug("upnp:event", request.getHeader().toString());
				onEventSubscriptionRequest(request, response, uri);
				return;
			}

			response.setStatusCode(404);
			response.setContentType("text/plain");
			setFixedTransfer(response, "Not found");
		}

		void onDeviceDescriptionRequest(HttpRequest & request, HttpResponse & response, const string & uri) {
			AutoRef<UPnPDeviceProfileSession> session = server.getProfileManager().getDeviceProfileSessionByUuid(request.getParameter("udn"));
			if (!session->isEnabled()) {
				response.setStatusCode(404);
				return;
			}
				
			response.setStatusCode(200);
			response.setContentType("text/xml");
			UPnPDeviceProfile profile = server.getProfileManager().getDeviceProfileSessionByUuid(request.getParameter("udn"))->profile();

			setFixedTransfer(response, profile.deviceDescription());
		}
		
		void onScpdRequest(HttpRequest & request, HttpResponse & response, const string & uri) {
			UPnPDeviceProfile deviceProfile = server.getProfileManager().getDeviceProfileSessionHasScpdUrl(uri)->profile();
			response.setStatusCode(200);
			response.setContentType("text/xml");
			UPnPServiceProfile serviceProfile = deviceProfile.getServiceProfileByScpdUrl(uri);
			setFixedTransfer(response, serviceProfile.scpd());
		}
		
		void onControlRequest(HttpRequest & request, HttpResponse & response, const string & uri) {
			// TODO: recognize specific device and service
			UPnPActionRequest actionRequest = parseActionRequest(request);
			UPnPActionResponse actionResponse;
			actionResponse.actionName() = actionRequest.actionName();
			actionResponse.serviceType() = actionRequest.serviceType();
			handleActionRequest(actionRequest, actionResponse);

			response.setStatusCode(200);
			response.setContentType("text/xml");
			setFixedTransfer(response, makeSoapResponseContent(actionResponse));
		}
		
		void onEventSubscriptionRequest(HttpRequest & request, HttpResponse & response, const string & uri) {
			
			UPnPDeviceProfile deviceProfile = server.getProfileManager().getDeviceProfileSessionHasEventSubUrl(uri)->profile();
			UPnPServiceProfile serviceProfile = deviceProfile.getServiceProfileByEventSubUrl(uri);

			if (request.getMethod() == "SUBSCRIBE") {

				if (request.getHeaderFieldIgnoreCase("SID").empty()) {
					vector<string> callbacks = server.parseCallbackUrls(request.getHeaderFieldIgnoreCase("CALLBACK"));
					unsigned long timeout = server.parseTimeout(request.getHeaderFieldIgnoreCase("TIMEOUT"));
					string sid = server.onSubscribe(deviceProfile, serviceProfile, callbacks, timeout);
					response.setStatusCode(200);
					response.setContentType("text/xml");
					response.getHeader().setHeaderField("SID", sid);
					// TODO: set TIMEOUT (Second-n, n should be greater than or equal to 1800)
				} else {
					string sid = request.getHeaderFieldIgnoreCase("SID");
					unsigned long timeout = server.parseTimeout(request.getHeaderFieldIgnoreCase("TIMEOUT"));
					server.onRenewSubscription(sid, timeout);
					response.setStatusCode(200);
				}
			} else if (request.getMethod() == "UNSUBSCRIBE") {
				server.onUnsubscribe(request.getHeaderFieldIgnoreCase("SID"));
				response.setStatusCode(200);
			} else {
				throw Exception("wrong event subscription request");
			}
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
				throw Exception("wrong soap action header format", -1, 0);
			}
			string serviceType = soapAction.substr(0, f);
			string actionName = soapAction.substr(f + 1);

			actionRequest.serviceType() = serviceType;
			actionRequest.actionName() = actionName;

			string xml = ((StringDataSink*)&request.getTransfer()->sink())->data();
			XML::XmlDocument doc = XML::DomParser::parse(xml);
			if (doc.getRootNode().nil()) {
				throw Exception("wrong soap action xml format", -1, 0);
			}

			XML::XmlNode * actionNode = doc.getRootNode()->getElementByTagName(actionName);
			if (actionNode == NULL) {
				throw Exception("wrong soap action xml format / no action name tag", -1, 0);
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

	string UPnPServer::DEFAULT_SERVER_INFO = "Cross-Platform/0 UPnP/1.0 App/0";
	
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
		timerThread.looper().interval(15 * 1000, AutoRef<TimerTask>(new UPnPServerLifetimeTask(*this)));

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
		url.setParameter("udn", deviceProfile.uuid());
		return url.toString();
	}

	void UPnPServer::setEnableDevice(const string & udn, bool enable) {
		Uuid uuid(udn);
		getProfileManager().getDeviceProfileSessionByUuid(uuid.getUuid())->setEnable(enable);
		if (enable) {
			notifyAlive(getProfileManager().getDeviceProfileSessionByUuid(uuid.getUuid())->profile());
		} else {
			notifyByeBye(getProfileManager().getDeviceProfileSessionByUuid(uuid.getUuid())->profile());
		}
	}

	void UPnPServer::setEnableAllDevices(bool enable) {
		vector<AutoRef<UPnPDeviceProfileSession> > profiles = getProfileManager().sessionList();
		for (vector<AutoRef<UPnPDeviceProfileSession> >::iterator iter = profiles.begin(); iter != profiles.end(); iter++) {
			(*iter)->setEnable(enable);
			if (enable) {
				notifyAlive((*iter)->profile());
			} else {
				notifyByeBye((*iter)->profile());
			}
		}
	}

	void UPnPServer::notifyAliveAll() {
		vector<AutoRef<UPnPDeviceProfileSession> > sessions = profileManager.sessionList();
		for (vector<AutoRef<UPnPDeviceProfileSession> >::iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
			if ((*iter)->isEnabled()) {
				notifyAlive((*iter)->profile());
			}
		}
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
	void UPnPServer::notifyAliveByDeviceType(UPnPDeviceProfile & profile, const string & deviceType) {

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
			"Server: " + config.getProperty("server.info", DEFAULT_SERVER_INFO) + "\r\n"
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
	void UPnPServer::notifyByeByeByDeviceType(UPnPDeviceProfile & profile, const string & deviceType) {

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
			"Server: " + config.getProperty("server.info", DEFAULT_SERVER_INFO) + "\r\n"
			"USN: uuid:" + uuid + "::" + st + "\r\n"
			"Ext: \r\n"
			"\r\n";
	}

	UPnPDeviceProfileSessionManager & UPnPServer::getProfileManager() {
		return profileManager;
	}

	void UPnPServer::registerDeviceProfile(const Url & url) {
		UPnPDeviceDeserializer deserializer;
		UPnPDeviceProfileBuilder builder(deserializer.build(url));
		UPnPDeviceProfile profile = builder.build();
		registerDeviceProfile(profile);
	}
	
	void UPnPServer::registerDeviceProfile(const string & uuid, const Url & url) {
		UPnPDeviceDeserializer deserializer;
		UPnPDeviceProfileBuilder builder(uuid, deserializer.build(url));
		UPnPDeviceProfile profile = builder.build();
		registerDeviceProfile(profile);
	}
	
	void UPnPServer::registerDeviceProfile(const UPnPDeviceProfile & profile) {
		getProfileManager().registerProfile(profile);
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

	void UPnPServer::setProperties(const string & udn, const string & serviceType, LinkedStringMap & props) {
		getPropertyManager().setProperties(udn, serviceType, props);
	}

	void UPnPServer::notifyEvent(const string & sid) {
		notificationThread.notify(sid);
	}
	
	void UPnPServer::delayNotifyEvent(const string & sid, unsigned long delay) {
		notificationThread.delayNotify(sid, delay);
	}

	string UPnPServer::onSubscribe(const UPnPDeviceProfile & device, const UPnPServiceProfile & service, const vector<string> & callbacks, unsigned long timeout) {
		
		UuidGeneratorVersion1 gen;
		string sid = gen.generate();

		AutoRef<UPnPEventSubscriptionSession> session(new UPnPEventSubscriptionSession);
		session->sid() = sid;
		session->callbackUrls() = callbacks;
		session->prolong(timeout);
		session->udn() = device.const_uuid();
		session->serviceType() = service.const_serviceType();

		propertyManager.addSubscriptionSession(session);

		delayNotifyEvent(sid, 100);

		return sid;
	}

	void UPnPServer::onRenewSubscription(const string & sid, unsigned long timeout) {
		propertyManager.getSession(sid)->prolong(timeout);
	}
	
	void UPnPServer::onUnsubscribe(const string & sid) {
		propertyManager.removeSubscriptionSession(sid);
	}

	vector<string> UPnPServer::parseCallbackUrls(const string & urls) {
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

	unsigned long UPnPServer::parseTimeout(const string & phrase) {
		if (Text::startsWithIgnoreCase(phrase, "Second-")) {
			return Text::toLong(phrase.substr(string("Second-").size())) * 1000;
		}
		return 0;
	}

	TimerLooperThread & UPnPServer::getTimerThread() {
		return timerThread;
	}

	void UPnPServer::collectOutdated() {
		propertyManager.collectOutdated();
	}

}
