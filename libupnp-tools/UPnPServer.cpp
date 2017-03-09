#include <liboslayer/XmlParser.hpp>
#include <liboslayer/MessageQueue.hpp>
#include <liboslayer/Uuid.hpp>
#include <libhttp-server/StringDataSink.hpp>
#include <libhttp-server/WebServerUtil.hpp>
#include "UPnPActionErrorCodes.hpp"
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
	vector<AutoRef<UPnPDeviceProfileSession> > UPnPDeviceProfileSessionManager::sessionList() {
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

	vector<string> UPnPDeviceProfileSessionManager::getAllTypes() {
		vector<string> types;
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin();
			 iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> & session = iter->second;
			Uuid uuid(session->profile().uuid());
			string udn = uuid.toString();
			types.push_back(udn);
			uuid.setRest("upnp:rootdevice");
			types.push_back(uuid.toString());
			vector<string> & deviceTypes = session->profile().deviceTypes();
			for (vector<string>::iterator iter = deviceTypes.begin(); iter != deviceTypes.end(); iter++) {
				uuid.setRest(*iter);
				types.push_back(uuid.toString());
			}
			vector<UPnPServiceProfile> & serviceProfiles = session->profile().serviceProfiles();
			for (vector<UPnPServiceProfile>::iterator iter = serviceProfiles.begin(); iter != serviceProfiles.end(); iter++) {
				uuid.setRest(iter->serviceType());
				types.push_back(uuid.toString());
			}
			
		}
		return types;
	}
	
	vector<string> UPnPDeviceProfileSessionManager::getTypes(const string & st) {
		vector<string> types;
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin();
			 iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> & session = iter->second;
			Uuid uuid(session->profile().uuid());
			string udn = uuid.toString();
			if (st == "upnp:rootdevice") {
				uuid.setRest("upnp:rootdevice");
				types.push_back(uuid.toString());
				continue;
			}
			if (Text::startsWith(st, "uuid:") && udn == Uuid(st).toString()) {
				types.push_back(udn);
				continue;
			}
			if (session->profile().hasDeviceType(st)) {
				uuid.setRest(st);
				types.push_back(uuid.toString());
				continue;
			}
			if (session->profile().hasServiceByServiceType(st)) {
				uuid.setRest(st);
				types.push_back(uuid.toString());
				continue;
			}
		}
		return types;
	}

	bool UPnPDeviceProfileSessionManager::hasDeviceProfileSessionByUuid(const string & uuid) {
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin();
			 iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> session = iter->second;
			if (session->profile().uuid() == uuid) {
				return true;
			}
		}
		return false;
	}
	
	bool UPnPDeviceProfileSessionManager::hasDeviceProfileSessionByScpdUrl(const string & scpdUrl) {
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin();
			 iter != _sessions.end(); iter++) {
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
		UPnPServerLifetimeTask(UPnPServer & server) : server(server) { /**/ }
		virtual ~UPnPServerLifetimeTask() { /**/ }
		virtual void doTask() {
			server.notifyAliveAll();
			server.collectOutdated();
		}
	};

	/**
	 * @brief notify task
	 */
	class NotifyTask : public TimerTask {
	public:
		static const int NOTIFY_NONE = 0;
		static const int NOTIFY_ALIVE = 1;
		static const int NOTIFY_ALIVE_ALL = 2;
		static const int NOTIFY_BYEBYE = 3;
		
	private:
		UPnPServer & server;
		int type;
		UPnPDeviceProfile profile;

	public:
		NotifyTask(UPnPServer & server, int type, const UPnPDeviceProfile & profile)
			: server(server), type(type), profile(profile)
			{ /**/ }
		virtual ~NotifyTask() { /**/ }
		virtual void doTask() {
			switch (type) {
			case NOTIFY_ALIVE:
				server.notifyAlive(profile);
				break;
			case NOTIFY_ALIVE_ALL:
				server.notifyAliveAll();
				break;
			case NOTIFY_BYEBYE:
				server.notifyByeBye(profile);
				break;
			default:
				break;
			}
		}
	};


	/**
	 * @brief ssdp handler
	 */
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

			if (header.getHeaderFieldIgnoreCase("MX").empty() ||
				header.getHeaderFieldIgnoreCase("MAN").empty()) {
				return;
			}
			
			InetAddress remoteAddr = header.getRemoteAddr();
			server.respondMsearch(header.getSt(), remoteAddr);
		}
	};

	/**
	 * @brief upnp server http request handler
	 */
	class UPnPServerHttpRequestHandler : public HttpRequestHandler, public WebServerUtil {
	private:
		UPnPServer & server;
		
	public:
		UPnPServerHttpRequestHandler(UPnPServer & server) : server(server) { /**/ }
		virtual ~UPnPServerHttpRequestHandler() { /**/ }

		virtual void onHttpRequestHeaderCompleted(HttpRequest & request, HttpResponse & response) {

			response.setContentLength(0);
			
			// https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
			if (request.getHeaderFieldIgnoreCase("Expect") == "100-continue") {
				request.clearTransfer();
				response.setStatus(100); // 100 continue, 417 expectation failed
			}
		}

		virtual void onHttpRequestContentCompleted(HttpRequest & request,
												   AutoRef<DataSink> sink,
												   HttpResponse & response) {

			if (response.getStatusCode() == 100 || response.getStatusCode() == 417) {
				return;
			}

			string uri = request.header().getPart2();
			if (request.getPath() == "/device.xml") {
				server.debug("upnp:device-description", request.header().toString());
				if (request.getMethod() != "GET") {
					response.setStatus(405);
					return;
				}
				prepareCommonResponse(request, response);
				onDeviceDescriptionRequest(request, response, uri);
				return;
			}
			
			if (server.getProfileManager().hasDeviceProfileSessionByScpdUrl(uri)) {
				server.debug("upnp:scpd", request.header().toString());
				if (request.getMethod() != "GET") {
					response.setStatus(405);
					return;
				}
				prepareCommonResponse(request, response);
				onScpdRequest(request, response, uri);
				return;
			}
			
			if (server.getProfileManager().hasDeviceProfileSessionByControlUrl(uri)) {
				server.debug("upnp:control", request.header().toString() +
							 (sink.nil() ? "" : ((StringDataSink*)&sink)->data()));
				if (request.getMethod() != "POST") {
					response.setStatus(405);
					return;
				}
				prepareCommonResponse(request, response);
				onControlRequest(request, response, uri);
				return;
			}

			if (server.getProfileManager().hasDeviceProfileSessionByEventSubUrl(uri)) {
				server.debug("upnp:event", request.header().toString() +
							 (sink.nil() ? "" : ((StringDataSink*)&sink)->data()));
				prepareCommonResponse(request, response);
				onEventSubscriptionRequest(request, response, uri);
				return;
			}

			response.setStatus(404);
			response.setContentType("text/plain");
			setFixedTransfer(response, "Not found");
		}

		void prepareCommonResponse(HttpRequest & request, HttpResponse & response) {
			response.header().setHeaderField("Ext", "");
			if (request.header().hasHeaderFieldIgnoreCase("ACCEPT-LANGUAGE")) {
				response.header().setHeaderField("CONTENT-LANGUAGE", "en");
			}
			response.header().setHeaderField("Server", server.getServerInfo());
			response.header().setHeaderField("Date", Date::formatRfc1123(Date::now()));
		}

		void onDeviceDescriptionRequest(HttpRequest & request, HttpResponse & response, const string & uri) {
			if (!server.getProfileManager().hasDeviceProfileSessionByUuid(request.getParameter("udn"))) {
				response.setStatus(404);
				return;
			}
			AutoRef<UPnPDeviceProfileSession> session = server.
				getProfileManager().
				getDeviceProfileSessionByUuid(request.getParameter("udn"));
			if (!session->isEnabled()) {
				response.setStatus(404);
				return;
			}
				
			response.setStatus(200);
			response.setContentType("text/xml; charset=\"utf-8\"");
			UPnPDeviceProfile profile = server.getProfileManager().
				getDeviceProfileSessionByUuid(request.getParameter("udn"))->profile();

			setFixedTransfer(response, profile.deviceDescription());
		}
		
		void onScpdRequest(HttpRequest & request, HttpResponse & response, const string & uri) {
			UPnPDeviceProfile deviceProfile = server.getProfileManager().getDeviceProfileSessionHasScpdUrl(uri)->profile();
			response.setStatus(200);
			response.setContentType("text/xml; charset=\"utf-8\"");
			UPnPServiceProfile serviceProfile = deviceProfile.getServiceProfileByScpdUrl(uri);
			setFixedTransfer(response, serviceProfile.scpd());
		}
		
		void onControlRequest(HttpRequest & request, HttpResponse & response, const string & uri) {
			// TODO: recognize specific device and service
			UPnPActionRequest actionRequest = parseActionRequest(request);
			UPnPActionResponse actionResponse;
			actionResponse.actionName() = actionRequest.actionName();
			actionResponse.serviceType() = actionRequest.serviceType();
			if (handleActionRequest(actionRequest, actionResponse)) {
				if (actionResponse.errorCode() == 0) {
					response.setStatus(200);
					response.setContentType("text/xml; charset=\"utf-8\"");
					setFixedTransfer(response, makeSoapResponseContent(actionResponse));
				} else {
					response.setStatus(500);
					response.setContentType("text/xml; charset=\"utf-8\"");
					setFixedTransfer(response, makeSoapErrorResponseContent(actionResponse.errorCode(),
																			actionResponse.errorString()));
				}
			} else {
				response.setStatus(500);
				response.setContentType("text/xml; charset=\"utf-8\"");
				setFixedTransfer(response, makeSoapErrorResponseContent(401));
			}
		}
		
		void onEventSubscriptionRequest(HttpRequest & request, HttpResponse & response, const string & uri) {
			UPnPDeviceProfile
				deviceProfile
				= server.getProfileManager().getDeviceProfileSessionHasEventSubUrl(uri)->
				profile();
			UPnPServiceProfile serviceProfile = deviceProfile.getServiceProfileByEventSubUrl(uri);
			if (request.getMethod() == "SUBSCRIBE") {
				if (!request.getHeaderFieldIgnoreCase("SID").empty() &&
					(!request.getHeaderFieldIgnoreCase("NT").empty() ||
					 !request.getHeaderFieldIgnoreCase("CALLBACK").empty())) {
					response.setStatus(400, "Incompatible header fields");
					return;
				}
				if (request.getHeaderFieldIgnoreCase("SID").empty()) {
					if (request.getHeaderFieldIgnoreCase("CALLBACK").empty() ||
						request.getHeaderFieldIgnoreCase("NT") != "upnp:event") {
						response.setStatus(412, "Precondition Failed");
						return;
					}
					vector<string> callbacks;
					try {
						callbacks = server.
							parseCallbackUrls(request.getHeaderFieldIgnoreCase("CALLBACK"));
					} catch (Exception e) {
						response.setStatus(412, "Precondition Failed");
						return;
					}
					unsigned long
						timeoutMilli = server.
						parseTimeoutMilli(request.getHeaderFieldIgnoreCase("TIMEOUT"));
					if (timeoutMilli <= 1800 * 1000) {
						timeoutMilli = 1800 * 1000;
					}
					string
						sid = server.
						onSubscribe(deviceProfile, serviceProfile, callbacks, timeoutMilli);
					response.setStatus(200);
					response.header().setHeaderField("SID", sid);
					response.header().setHeaderField("TIMEOUT",
														"Second-" + Text::toString(timeoutMilli / 1000));
					server.delayNotifyEvent(sid, 500);
				} else { // re-subscribe
					string
						sid = request.getHeaderFieldIgnoreCase("SID");
					unsigned long
						timeoutMilli = server.parseTimeoutMilli(request.getHeaderFieldIgnoreCase("TIMEOUT"));
					if (timeoutMilli <= 1800 * 1000) {
						timeoutMilli = 1800 * 1000;
					}
					if (server.onRenewSubscription(sid, timeoutMilli)) {
						response.setStatus(200);
						response.header().setHeaderField("TIMEOUT",
															"Second-" + Text::toString(timeoutMilli / 1000));
						server.delayNotifyEvent(sid, 500);
					} else {
						response.setStatus(412, "Precondition Failed");
					}
				}
			} else if (request.getMethod() == "UNSUBSCRIBE") {
				if (!request.getHeaderFieldIgnoreCase("SID").empty() &&
					(!request.getHeaderFieldIgnoreCase("NT").empty() ||
					 !request.getHeaderFieldIgnoreCase("CALLBACK").empty())) {
					response.setStatus(400, "Incompatible header fields");
					return;
				}
				if (server.onUnsubscribe(request.getHeaderFieldIgnoreCase("SID"))) {
					response.setStatus(200);
				} else {
					response.setStatus(412, "Precondition Failed");
				}
				
			} else {
				throw Exception("wrong event subscription request");
			}
		}

		bool handleActionRequest(UPnPActionRequest & request, UPnPActionResponse & response) {
			if (!server.getActionRequestHandler().nil()) {
				return server.getActionRequestHandler()->handleActionRequest(request, response);
			}
			return false;
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

			AutoRef<XML::XmlNode> actionNode = doc.getRootNode()->getElementByTagName(actionName);
			if (actionNode.nil()) {
				throw Exception("wrong soap action xml format / no action name tag", -1, 0);
			}

			vector<AutoRef<XML::XmlNode> > children = actionNode->children();
			for (vector<AutoRef<XML::XmlNode> >::iterator iter = children.begin(); iter != children.end(); iter++) {
				if (XmlUtils::testNameValueXmlNode(*iter)) {
					NameValue nv = XmlUtils::toNameValue(*iter);
					actionRequest[nv.name()] = nv.value();
				}
			}

			return actionRequest;
		}

		string makeSoapResponseContent(UPnPActionResponse & response) {

			string actionName = response.actionName();
			string serviceType = response.serviceType();
		    LinkedStringMap & parameters = response.parameters();
			
			string xml;
			xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n";
			xml.append("<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" "
					   "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n");
			xml.append("<s:Body>\r\n");
			xml.append("<u:" + actionName + "Response xmlns:u=\"" + serviceType + "\">\r\n");
			for (size_t i = 0; i < parameters.size(); i++) {
				NameValue & nv = parameters[i];
				string name = XML::XmlEncoder::encode(nv.name());
				string value = XML::XmlEncoder::encode(nv.value());
				xml.append("<" + name + ">" + value + "</" + name + ">\r\n");
			}
			xml.append("</u:" + actionName + "Response>");
			xml.append("</s:Body>\r\n");
			xml.append("</s:Envelope>");
			return xml;
		}

		string makeSoapErrorResponseContent(int errorCode) {
			return makeSoapErrorResponseContent(errorCode, UPnPActionErrorCodes::getDescription(errorCode));
		}

		string makeSoapErrorResponseContent(int errorCode, const string & errorString) {
			string xml;
			xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n";
			xml.append("<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" "
					   "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n");
			xml.append("<s:Body>\r\n");
			xml.append("<s:Fault>");
			xml.append("<faultcode>s:Client</faultcode>");
			xml.append("<faultstring>UPnPError</faultstring>");
			xml.append("<detail>");
			xml.append("<UPnPError xmlns=\"urn:schemas-upnp-org:control-1-0\">");
			xml.append("<errorCode>" + Text::toString(errorCode) + "</errorCode>");
			xml.append("<errorDescription>" + errorString + "</errorDescription>");
			xml.append("</UPnPError>");
			xml.append("</detail>");
			xml.append("</s:Fault>");
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

	string UPnPServer::DEFAULT_SERVER_INFO = "Cross-Platform/0.0 UPnP/1.0 App/0.0";
	
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
		timerThread.looper().
			interval(15 * 1000, AutoRef<TimerTask>(new UPnPServerLifetimeTask(*this)));

		ssdpServer.addSSDPEventListener(ssdpListener);
		ssdpServer.startAsync();
	}
	
	void UPnPServer::stop() {
		
		if (!httpServer) {
			return;
		}

		ssdpServer.stop();

		timerThread.stop();
		timerThread.wait();
		
		notificationThread.interrupt();
		notificationThread.wait();
		
		httpServer->stop();
		delete httpServer;
		httpServer = NULL;
	}

	string UPnPServer::makeLocation(const string & uuid) {
		Url url;
		url.setProtocol("http");
		url.setHost(NetworkUtil::selectDefaultAddress().getHost());
		url.setPort(config["listen.port"]);
		url.setPath("device.xml");
		url.setParameter("udn", uuid);
		return url.toString();
	}

	void UPnPServer::setEnableDevice(const string & udn, bool enable) {
		
		Uuid uuid(udn);
		
		getProfileManager().getDeviceProfileSessionByUuid(uuid.getUuid())->
			setEnable(enable);
		
		if (enable) {
			notifyAlive(getProfileManager().
						getDeviceProfileSessionByUuid(uuid.getUuid())->
						profile());
		} else {
			notifyByeBye(getProfileManager().
						 getDeviceProfileSessionByUuid(uuid.getUuid())->
						 profile());
		}
	}

	void UPnPServer::setEnableAllDevices(bool enable) {
		vector<AutoRef<UPnPDeviceProfileSession> > profiles = getProfileManager().sessionList();
		for (vector<AutoRef<UPnPDeviceProfileSession> >::iterator iter = profiles.begin(); iter != profiles.end(); iter++) {
			(*iter)->setEnable(enable);
			if (enable) {
				delayNotify(0, NotifyTask::NOTIFY_ALIVE, (*iter)->profile());
				delayNotify(200, NotifyTask::NOTIFY_ALIVE, (*iter)->profile());
			} else {
				delayNotify(0, NotifyTask::NOTIFY_BYEBYE, (*iter)->profile());
				delayNotify(200, NotifyTask::NOTIFY_BYEBYE, (*iter)->profile());
			}
		}
	}

	void UPnPServer::delayNotify(unsigned long delay, int type, const UPnPDeviceProfile & profile) {
		timerThread.looper().delay(delay, AutoRef<TimerTask>(new NotifyTask(*this, type, profile)));
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
		string location = makeLocation(profile.uuid());
		sender.sendMcastToAllInterfaces(makeNotifyAlive(location, uuid, "upnp:rootdevice"),
										host, port);
		sender.sendMcastToAllInterfaces(makeNotifyAlive(location, uuid, ""),
										host, port);
		vector<string> & deviceTypes = profile.deviceTypes();
		for (vector<string>::iterator iter = deviceTypes.begin();
			 iter != deviceTypes.end(); iter++) {
			sender.sendMcastToAllInterfaces(makeNotifyAlive(location, uuid, *iter),	host, port);
		}
		vector<UPnPServiceProfile> & serviceProfiles = profile.serviceProfiles();
		for (vector<UPnPServiceProfile>::iterator iter = serviceProfiles.begin();
			 iter != serviceProfiles.end(); iter++) {
			sender.sendMcastToAllInterfaces(makeNotifyAlive(location, uuid, iter->serviceType()), host, port);
		}
		sender.close();
	}
	
	void UPnPServer::notifyAliveByDeviceType(UPnPDeviceProfile & profile, const string & deviceType) {

		string uuid = profile.uuid();
		string location = makeLocation(profile.uuid());
		
		SSDPMsearchSender sender;
		string packet = makeNotifyAlive(location, uuid, deviceType);
		debug("ssdp:notify-alive", packet);
		sender.sendMcastToAllInterfaces(packet, "239.255.255.250", 1900);
		sender.close();
	}
	
	string UPnPServer::makeNotifyAlive(const string & location, const string & uuid, const string & deviceType) {

		Uuid u(uuid);
		u.setRest(deviceType);
		
		return "NOTIFY * HTTP/1.1\r\n"
			"Cache-Control: max-age=1800\r\n"
			"HOST: 239.255.255.250:1900\r\n"
			"Location: " + location + "\r\n"
			"NT: " + (deviceType.empty() ? u.toString() : deviceType) + "\r\n"
			"NTS: ssdp:alive\r\n"
			"Server: " + config.getProperty("server.info", DEFAULT_SERVER_INFO) + "\r\n"
			"USN: " + u.toString() + "\r\n"
			"\r\n";
	}
	void UPnPServer::notifyByeBye(UPnPDeviceProfile & profile) {

		SSDPMsearchSender sender;
		string host = "239.255.255.250";
		int port = 1900;

		string uuid = profile.uuid();
		string location = makeLocation(profile.uuid());
		sender.sendMcastToAllInterfaces(makeNotifyByeBye(uuid, "upnp:rootdevice"), host, port);
		sender.sendMcastToAllInterfaces(makeNotifyByeBye(uuid, ""), host, port);

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
		string packet = makeNotifyByeBye(uuid, deviceType);
		debug("ssdp:notify-byebye", packet);
		sender.sendMcastToAllInterfaces(packet, "239.255.255.250", 1900);
		sender.close();
	}
	string UPnPServer::makeNotifyByeBye(const string & uuid, const string & deviceType) {

		Uuid u(uuid);
		u.setRest(deviceType);
		
		return "NOTIFY * HTTP/1.1\r\n"
			"Host: 239.255.255.250:1900\r\n"
			"NT: " + (deviceType.empty() ? u.toString() : deviceType)  + "\r\n"
			"NTS: ssdp:byebye\r\n"
			"USN: " + u.toString() + "\r\n"
			"\r\n";
	}

	void UPnPServer::respondMsearch(const string & st, InetAddress & remoteAddr) {
		SSDPMsearchSender sender;

		vector<string> types = ((st == "ssdp:all") ?
								getProfileManager().getAllTypes() :
								getProfileManager().getTypes(st));

		for (vector<string>::iterator iter = types.begin(); iter != types.end(); iter++) {
			Uuid uuid(*iter);
			string location = makeLocation(uuid.getUuid());
			string packet = makeMsearchResponse(location, uuid.getUuid(), uuid.getRest());
			debug("ssdp:response-msearch", packet);
			sender.unicast(packet, remoteAddr);
		}
		sender.close();
	}
	string UPnPServer::makeMsearchResponse(const string & location, const string & uuid, const string & st) {

		Uuid u(uuid);
		u.setRest(st);
		
		return "HTTP/1.1 200 OK\r\n"
			"Cache-Control: max-age=1800\r\n"
			"HOST: 239.255.255.250:1900\r\n"
			"Location: " + location + "\r\n"
			"ST: " + (st.empty() ? u.toString() : st) + "\r\n"
			"Server: " + config.getProperty("server.info", DEFAULT_SERVER_INFO) + "\r\n"
			"USN: " + u.toString() + "\r\n"
			"Ext: \r\n"
			"Date: " + Date::formatRfc1123(Date::now()) + "\r\n"
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

	void UPnPServer::unregisterDeviceProfile(const string & uuid) {
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

	string UPnPServer::onSubscribe(const UPnPDeviceProfile & device,
								   const UPnPServiceProfile & service,
								   const vector<string> & callbacks,
								   unsigned long timeout) {
		
		UuidGeneratorVersion1 gen;
		string sid = Uuid(gen.generate()).toString();

		AutoRef<UPnPEventSubscriptionSession> session(new UPnPEventSubscriptionSession);
		session->sid() = sid;
		session->callbackUrls() = callbacks;
		session->prolong(timeout);
		session->udn() = device.const_uuid();
		session->serviceType() = service.const_serviceType();

		propertyManager.addSubscriptionSession(session);

		return sid;
	}

	bool UPnPServer::onRenewSubscription(const string & sid, unsigned long timeout) {
		if (propertyManager.hasSubscriptionSession(sid)) {
			propertyManager.getSession(sid)->prolong(timeout);
			return true;
		}
		return false;
	}
	
	bool UPnPServer::onUnsubscribe(const string & sid) {
		if (propertyManager.hasSubscriptionSession(sid)) {
			propertyManager.removeSubscriptionSession(sid);
			return true;
		}
		return false;
	}

	vector<string> UPnPServer::parseCallbackUrls(const string & urls) {
		vector<string> ret;
		string buffer;
		if (urls.empty()) {
			throw Exception("empty callback string");
		}
		for (string::const_iterator iter = urls.begin(); iter != urls.end(); iter++) {
			if (*iter == '<') {
				buffer = "";
				for (iter++; iter != urls.end() && *iter != '>'; iter++) {
					buffer.append(1, *iter);
				}
				Url url(buffer); // test valid url
				ret.push_back(buffer);
			}
		}
		if (ret.empty()) {
			throw Exception("no parsed callback - maybe wrong format");
		}
		return ret;
	}

	unsigned long UPnPServer::parseTimeoutMilli(const string & phrase) {
		unsigned long timeout = 0;
		if (Text::startsWithIgnoreCase(phrase, "Second-")) {
			timeout = (unsigned long)Text::toLong(phrase.substr(string("Second-").size())) * 1000;
		}
		return timeout;
	}

	TimerLooperThread & UPnPServer::getTimerThread() {
		return timerThread;
	}

	void UPnPServer::collectOutdated() {
		propertyManager.collectOutdated();
	}

	string UPnPServer::getServerInfo() {
		return config.getProperty("server.info", DEFAULT_SERVER_INFO);
	}
}
