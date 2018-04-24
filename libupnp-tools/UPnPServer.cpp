#include <liboslayer/XmlParser.hpp>
#include <liboslayer/MessageQueue.hpp>
#include <liboslayer/Logger.hpp>
#include <liboslayer/Uuid.hpp>
#include <libhttp-server/StringDataSink.hpp>
#include <libhttp-server/HttpException.hpp>
#include "UPnPActionErrorCodes.hpp"
#include "UPnPDeviceDeserializer.hpp"
#include "UPnPDeviceBuilder.hpp"
#include "UPnPServer.hpp"
#include "SSDPMsearchSender.hpp"
#include "NetworkUtil.hpp"
#include "UPnPActionRequest.hpp"
#include "UPnPActionResponse.hpp"
#include "UPnPSoapFormatter.hpp"
#include "UPnPSoapException.hpp"
#include "UPnPDeviceSerializer.hpp"
#include "UPnPTerms.hpp"
#include "XmlUtils.hpp"
#include <liboslayer/File.hpp>
#include <liboslayer/Logger.hpp>


namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace SSDP;
	using namespace HTTP;
	using namespace UTIL;
	

	static AutoRef<Logger> logger = LoggerFactory::instance().
		getObservingLogger(File::basename(__FILE__));


	UPnPDeviceProfileManager::UPnPDeviceProfileManager() {
	}

	UPnPDeviceProfileManager::~UPnPDeviceProfileManager() {
	}

	vector< AutoRef<UPnPDeviceProfile> > & UPnPDeviceProfileManager::profiles() {
		return _profiles;
	}

	void UPnPDeviceProfileManager::registerProfile(AutoRef<UPnPDeviceProfile> profile) {
		for (vector< AutoRef<UPnPDeviceProfile> >::iterator iter = _profiles.begin();
			 iter != _profiles.end(); iter++)
		{
			if ((*iter)->udn() == profile->udn()) {
				throw Exception("Already registered");
			}
		}
		_profiles.push_back(profile);
	}

	void UPnPDeviceProfileManager::unregisterProfile(const UDN & udn) {
		for (vector< AutoRef<UPnPDeviceProfile> >::iterator iter = _profiles.begin();
			 iter != _profiles.end(); iter++)
		{
			if ((*iter)->udn() == udn) {
				_profiles.erase(iter);
			}
		}
	}

	vector<string> UPnPDeviceProfileManager::getAllTypes() {
		vector<string> types;
		for (vector< AutoRef<UPnPDeviceProfile> >::iterator iter = _profiles.begin();
			 iter != _profiles.end(); iter++) {
			AutoRef<UPnPDeviceProfile> profile = (*iter);
			USN usn(profile->udn().toString());
			types.push_back(usn.toString());
			usn.rest() = "upnp:rootdevice";
			types.push_back(usn.toString());
			vector<string> deviceTypes = profile->deviceTypes();
			for (vector<string>::iterator iter = deviceTypes.begin();
				 iter != deviceTypes.end(); iter++)
			{
				usn.rest() = (*iter);
				types.push_back(usn.toString());
			}
			vector<string> serviceTypes = profile->serviceTypes();
			for (vector<string>::iterator iter = serviceTypes.begin();
				 iter != serviceTypes.end(); iter++)
			{
				usn.rest() = (*iter);
				types.push_back(usn.toString());
			}
		}
		return types;
	}
	
	vector<string> UPnPDeviceProfileManager::getTypes(const string & st) {
		vector<string> types;
		for (vector< AutoRef<UPnPDeviceProfile> >::iterator iter = _profiles.begin();
			 iter != _profiles.end(); iter++)
		{
			AutoRef<UPnPDeviceProfile> profile = (*iter);
			USN usn(profile->udn().toString());
			if (st == "upnp:rootdevice") {
				usn.rest() = "upnp:rootdevice";
				types.push_back(usn.toString());
				continue;
			}
			if (Text::startsWith(st, "uuid:") && usn.uuid() == USN(st).uuid()) {
				types.push_back("uuid:" + usn.uuid());
				continue;
			}

			bool found = false;
			vector<string> deviceTypes = profile->deviceTypes();
			for (vector<string>::iterator iter = deviceTypes.begin();
				 iter != deviceTypes.end(); ++iter)
			{
				if (*iter == st) {
					usn.rest() = st;
					types.push_back(usn.toString());
					found = true;
					break;
				}
			}
			if (found) {
				continue;
			}

			vector<string> serviceTypes = profile->serviceTypes();
			for (vector<string>::iterator iter = serviceTypes.begin();
				 iter != serviceTypes.end(); ++iter)
			{
				if (*iter == st) {
					usn.rest() = st;
					types.push_back(usn.toString());
					break;
				}
			}
		}
		return types;
	}

	
	AutoRef<UPnPDeviceProfile> UPnPDeviceProfileManager::getDeviceProfile(const UDN & udn) {
		for (vector< AutoRef<UPnPDeviceProfile> >::iterator iter = _profiles.begin();
			 iter != _profiles.end(); iter++)
		{
			AutoRef<UPnPDeviceProfile> profile = (*iter);
			if (profile->udn() == udn) {
				return profile;
			}
		}
		throw Exception("not found device profile session by udn");
	}
	
	AutoRef<UPnPDeviceProfile> UPnPDeviceProfileManager::getDeviceProfileHasScpdUrl(const string & scpdUrl) {
		for (vector< AutoRef<UPnPDeviceProfile> >::iterator iter = _profiles.begin();
			 iter != _profiles.end(); iter++)
		{
			AutoRef<UPnPDeviceProfile> profile = (*iter);
			vector< AutoRef<UPnPService> > services = profile->allServices();
			for (vector< AutoRef<UPnPService> >::iterator it = services.begin();
				 it != services.end(); ++it)
			{
				if ((*it)->scpdUrl() == scpdUrl) {
					return profile;
				}
			}
		}
		throw Exception("not found device profile session by scpd url - " + scpdUrl);
	}


	AutoRef<UPnPDeviceProfile> UPnPDeviceProfileManager::getDeviceProfileHasControlUrl(const string & controlUrl) {
		for (vector< AutoRef<UPnPDeviceProfile> >::iterator iter = _profiles.begin();
			 iter != _profiles.end(); iter++)
		{
			AutoRef<UPnPDeviceProfile> profile = (*iter);
			vector< AutoRef<UPnPService> > services = profile->allServices();
			for (vector< AutoRef<UPnPService> >::iterator it = services.begin();
				 it != services.end(); ++it)
			{
				if ((*it)->controlUrl() == controlUrl) {
					return profile;
				}
			}
		}
		throw Exception("not found device profile session by control url - " + controlUrl);
	}

	
	AutoRef<UPnPDeviceProfile> UPnPDeviceProfileManager::getDeviceProfileHasEventSubUrl(const string & eventSubUrl) {
		for (vector< AutoRef<UPnPDeviceProfile> >::iterator iter = _profiles.begin();
			 iter != _profiles.end(); iter++)
		{
			AutoRef<UPnPDeviceProfile> profile = (*iter);
			vector< AutoRef<UPnPService> > services = profile->allServices();
			for (vector< AutoRef<UPnPService> >::iterator it = services.begin();
				 it != services.end(); ++it)
			{
				if ((*it)->eventSubUrl() == eventSubUrl) {
					return profile;
				}
			}
		}
		throw Exception("not found device profile session by event sub url");
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
		virtual void onTask() {
			server.notifyAliveAll();
			server.collectExpired();
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
		AutoRef<UPnPDeviceProfile> profile;

	public:
		NotifyTask(UPnPServer & server, int type, AutoRef<UPnPDeviceProfile> profile)
			: server(server), type(type), profile(profile)
			{ /**/ }
		virtual ~NotifyTask() { /**/ }
		virtual void onTask() {
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
		virtual bool filter(const SSDPHeader & header) {
			UPnPDebug::instance().debug("ssdp", header.toString());
			return true;
		}
		/**
		 *  e.g.)
		 *
		 *  M-SEARCH * HTTP/1.1
		 *  HOST: 239.255.255.250:1900
		 *  MAN: "ssdp:discover"
		 *  MX: 3
		 *  ST: upnp:rootdevice
		 *  USER-AGENT: Android/23 UPnP/1.1 UPnPTool/1.4.7
		 *
		 *  M-SEARCH * HTTP/1.1
		 *  MX: 1
		 *  ST: upnp:rootdevice
		 *  MAN: "ssdp:discover"
		 *  User-Agent: UPnP/1.0 DLNADOC/1.50 Platinum/1.0.4.11
		 *  Host: 239.255.255.250:1900
		 *  Connection: close
		 */
		virtual void onMsearch(const SSDPHeader & header) {

			if (header.getHeaderField("MX").empty() ||
				header.getHeaderField("MAN").empty()) {
				return;
			}
			
			InetAddress remoteAddr = header.getRemoteAddr();
			server.respondMsearch(header.getSearchTarget(), remoteAddr);
		}
	};
	

	/**
	 * @brief upnp server http request handler
	 */
	class UPnPServerHttpRequestHandler : public HttpRequestHandler {
	private:
		UPnPServer & server;
	public:
		UPnPServerHttpRequestHandler(UPnPServer & server) : server(server) { /**/ }
		virtual ~UPnPServerHttpRequestHandler() { /**/ }

		virtual void onHttpRequestHeaderCompleted(HttpRequest & request, HttpResponse & response) {
			response.setContentLength(0);
			// @ref https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
			if (request.getHeaderField("Expect") == "100-continue") {
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
			
			try {
				if (handleUPnP(request, sink, response) == true) {
					return;
				}
			} catch (HttpException e) {
				response.setStatus(e.statusCode(), e.statusString());
				response.setContentType("text/plain");
				response.setFixedTransfer(e.description());
				return;
			}

			response.setStatus(404);
			response.setContentType("text/plain");
			response.setFixedTransfer("Not found");
		}

		bool handleUPnP(HttpRequest & request, AutoRef<DataSink> sink, HttpResponse & response) {
			if (request.getPath() == "/device.xml") {
				handleDeviceDescription(request, response);
				return true;
			}

			string path = request.getRawPath();

			try {

				AutoRef<UPnPDeviceProfile> profile = server.
					getProfileManager().getDeviceProfileHasScpdUrl(path);
				handleScpd(request, response, profile);
				return true;
			} catch (Exception e) {
			}

			try {
				server.getProfileManager().getDeviceProfileHasControlUrl(path);
				handleControl(request, sink, response);
				return true;
			} catch (Exception e) {
			}

			try {
				AutoRef<UPnPDeviceProfile> profile = server.
					getProfileManager().getDeviceProfileHasEventSubUrl(path);
				handleEvent(request, sink, response, profile);
				return true;
			} catch (Exception e) {
			}

			return false;
		}

		void handleDeviceDescription(HttpRequest & request, HttpResponse & response) {
			UPnPDebug::instance().debug("upnp:device-description", request.header().toString());
			validateMethod(request, "GET");
			prepareCommonResponse(request, response);
			onDeviceDescriptionRequest(request, response);
		}

		void handleScpd(HttpRequest & request, HttpResponse & response, AutoRef<UPnPDeviceProfile> profile) {
			UPnPDebug::instance().debug("upnp:scpd", request.header().toString());
			validateMethod(request, "GET");
			prepareCommonResponse(request, response);
			onScpdRequest(request, response, profile);
		}

		void handleControl(HttpRequest & request, AutoRef<DataSink> sink, HttpResponse & response) {
			UPnPDebug::instance().debug("upnp:control", request.header().toString() + (sink.nil() ? "" : ((StringDataSink*)&sink)->data()));
			validateMethod(request, "POST");
			prepareCommonResponse(request, response);
			onControlRequest(request, response);
		}

		void handleEvent(HttpRequest & request, AutoRef<DataSink> sink, HttpResponse & response, AutoRef<UPnPDeviceProfile> profile) {
			UPnPDebug::instance().debug("upnp:event", request.header().toString() +
						 (sink.nil() ? "" : ((StringDataSink*)&sink)->data()));
			prepareCommonResponse(request, response);
			onEventSubscriptionRequest(request, response, profile);
		}

		void validateMethod(HttpRequest & request, const string & expect) {
			if (request.getMethod() != expect) {
				logger->error("[UPnPServer] HTTP REQUEST / unexpected method - " + request.getMethod());
				throw HttpException(405, HttpStatusCodes::getStatusString(405),
									"Unexpected method - " + request.getMethod());
			}
		}

		void prepareCommonResponse(HttpRequest & request, HttpResponse & response) {
			response.setHeaderField("Ext", "");
			if (request.hasHeaderField("ACCEPT-LANGUAGE")) {
				response.setHeaderField("CONTENT-LANGUAGE", "en");
			}
			response.setHeaderField("Server", server.getServerInfo());
			response.setHeaderField("Date", Date::formatRfc1123(Date::now()));
		}

		void onDeviceDescriptionRequest(HttpRequest & request, HttpResponse & response) {
			try {
				UDN udn(request.getParameter("udn"));
				AutoRef<UPnPDeviceProfile> profile = server.getProfileManager().getDeviceProfile(udn);
				if (!profile->enabled()) {
					throw HttpException(404);
				}
				
				response.setStatus(200);
				response.setContentType("text/xml; charset=\"utf-8\"");
				string description = profile->deviceDescription();
				response.setFixedTransfer(description);

				if (server.getHttpEventListener().nil() == false) {
					server.getHttpEventListener()->onDeviceDescriptionRequest(request, response);
				}
            } catch (HttpException e) {
                throw e;
			} catch (Exception e) {
				logger->error(e.message());
				throw HttpException(404);
			}
		}
		
		void onScpdRequest(HttpRequest & request, HttpResponse & response, AutoRef<UPnPDeviceProfile> profile) {
			string path = request.getRawPath();
			response.setStatus(200);
			response.setContentType("text/xml; charset=\"utf-8\"");
			AutoRef<UPnPService> service = profile->device()->getServiceWithScpdUrl(path);
			if (service.nil()) {
				logger->error("service not found");
			}
			string scpd = UPnPDeviceSerializer::serializeScpd(service->scpd());
			response.setFixedTransfer(scpd);

			if (server.getHttpEventListener().nil() == false) {
				server.getHttpEventListener()->onScpdRequest(request, response);
			}
		}
		
		void onControlRequest(HttpRequest & request, HttpResponse & response) {
			// TODO: recognize specific device and service
			UPnPActionRequest actionRequest = parseActionRequest(request);
			UPnPActionResponse actionResponse;
			actionResponse.actionName() = actionRequest.actionName();
			actionResponse.serviceType() = actionRequest.serviceType();
			try {
				if (handleActionRequest(actionRequest, actionResponse) == false) {
					throw UPnPSoapException(401);
				}
				if (actionResponse.errorCode() != 0) {
					throw UPnPSoapException(actionResponse.errorCode(),
											actionResponse.errorString());
				}
				response.setStatus(200);
				response.setContentType("text/xml; charset=\"utf-8\"");
				response.setFixedTransfer(UPnPSoapFormatter::formatResponse(actionResponse));
			} catch (UPnPSoapException e) {
				response.setStatus(500);
				response.setContentType("text/xml; charset=\"utf-8\"");
				response.setFixedTransfer(e.getSoapErrorMessage());
			}

			if (server.getHttpEventListener().nil() == false) {
				server.getHttpEventListener()->onControlRequest(request, response);
			}
		}
		
		void onEventSubscriptionRequest(HttpRequest & request, HttpResponse & response, AutoRef<UPnPDeviceProfile> profile) {
			string path = request.getRawPath();
			AutoRef<UPnPService> service = profile->device()->getServiceWithEventSubUrl(path);
			if (request.getMethod() == "SUBSCRIBE") {
				if (!request.getHeaderField("SID").empty() &&
					(!request.getHeaderField("NT").empty() ||
					 !request.getHeaderField("CALLBACK").empty())) {
					response.setStatus(400, "Incompatible header fields");
					return;
				}
				if (request.getHeaderField("SID").empty()) {
					if (request.getHeaderField("CALLBACK").empty() ||
						request.getHeaderField("NT") != "upnp:event") {
						response.setStatus(412, "Precondition Failed");
						return;
					}
					vector<string> callbacks;
					try {
						CallbackUrls cbs = CallbackUrls::fromString(request.getHeaderField("CALLBACK"));
						callbacks = cbs.urls();
					} catch (Exception e) {
						response.setStatus(412, "Precondition Failed");
						return;
					}
					;
					Second second = Second::fromString(
						request.getHeaderField("TIMEOUT"));
					unsigned long timeoutMilli = second.milli();
					if (timeoutMilli <= 1800 * 1000) {
						timeoutMilli = 1800 * 1000;
					}
					string sid = server.
						onSubscribe(profile->device(), service, callbacks, timeoutMilli);
					response.setStatus(200);
					response.setHeaderField("SID", sid);
					response.setHeaderField("TIMEOUT", Second::toString(timeoutMilli / 1000));
					server.delayNotifyEvent(sid, 500);
				} else { // re-subscribe
					string sid = request.getHeaderField("SID");
					Second second = Second::fromString(
						request.getHeaderField("TIMEOUT"));
					unsigned long timeoutMilli = second.milli();
					if (timeoutMilli <= 1800 * 1000) {
						timeoutMilli = 1800 * 1000;
					}
					if (server.onRenewSubscription(sid, timeoutMilli)) {
						response.setStatus(200);
						response.setHeaderField("TIMEOUT",
												Second::toString(timeoutMilli / 1000));
						server.delayNotifyEvent(sid, 500);
					} else {
						response.setStatus(412, "Precondition Failed");
					}
				}
			} else if (request.getMethod() == "UNSUBSCRIBE") {
				if (!request.getHeaderField("SID").empty() &&
					(!request.getHeaderField("NT").empty() ||
					 !request.getHeaderField("CALLBACK").empty())) {
					response.setStatus(400, "Incompatible header fields");
					return;
				}
				if (server.onUnsubscribe(request.getHeaderField("SID"))) {
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
				throw Exception("wrong soap action header format");
			}
			string serviceType = soapAction.substr(0, f);
			string actionName = soapAction.substr(f + 1);
			actionRequest.serviceType() = serviceType;
			actionRequest.actionName() = actionName;
			string xml = ((StringDataSink*)&request.getTransfer()->sink())->data();
			XML::XmlDocument doc = XML::DomParser::parse(xml);
			if (doc.getRootNode().nil()) {
				throw Exception("wrong soap action xml format");
			}
			AutoRef<XML::XmlNode> actionNode = doc.getRootNode()->getElementByTagName(actionName);
			if (actionNode.nil()) {
				throw Exception("wrong soap action xml format / no action name tag");
			}
			vector<AutoRef<XML::XmlNode> > children = actionNode->children();
			for (vector<AutoRef<XML::XmlNode> >::iterator iter = children.begin();
				 iter != children.end(); iter++) {
				if (XmlUtils::testKeyValueXmlNode(*iter)) {
					KeyValue kv = XmlUtils::toKeyValue(*iter);
					actionRequest[kv.key()] = kv.value();
				}
			}
			return actionRequest;
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
		  notificationThread(propertyManager),
		  ssdpListener(new UPnPServerSsdpHandler(*this)) {
	}

	UPnPServer::UPnPServer(const UPnPServer::Config & config, AutoRef<NetworkStateManager> networkStateManager)
		: networkStateManager(networkStateManager),
		  config(config),
		  notificationThread(propertyManager),
		  ssdpListener(new UPnPServerSsdpHandler(*this)) {
	}
	
	UPnPServer::~UPnPServer() {
	}

	void UPnPServer::startAsync() {
		
		if (httpServer.nil() == false) {
			return;
		}
		
		HttpServerConfig httpServerConfig;
		httpServerConfig["listen.port"] = config["listen.port"];
		httpServerConfig["thread.count"] = config.getProperty("thread.count", "5");
		httpServer = AutoRef<AnotherHttpServer>(new AnotherHttpServer(httpServerConfig));
		httpServer->registerRequestHandler("/*",
										   AutoRef<HttpRequestHandler>(
											   new UPnPServerHttpRequestHandler(*this)));
		httpServer->startAsync();

		notificationThread.start();
		
		timerThread.start();
		timerThread.looper().interval(15 * 1000, AutoRef<TimerTask>(new UPnPServerLifetimeTask(*this)));

		ssdpServer.addSSDPEventListener(ssdpListener);
		ssdpServer.startAsync();
	}
	
	void UPnPServer::stop() {
		
		if (httpServer.nil() == true) {
			return;
		}

		ssdpServer.stop();

		timerThread.stop();
		timerThread.wait();
		
		notificationThread.interrupt();
		notificationThread.wait();
		
		httpServer->stop();
		httpServer = NULL;
	}

	AutoRef<AnotherHttpServer> UPnPServer::getHttpServer() {
		return httpServer;
	}

	string UPnPServer::makeLocation(const UDN & udn) {
		Url url;
		url.setProtocol("http");
		url.setHost(NetworkUtil::selectDefaultAddress().getHost());
		url.setPort(config["listen.port"]);
		url.setPath("device.xml");
		url.setParameter("udn", udn.toString());
		return url.toString();
	}

	void UPnPServer::activateDevice(const UDN & udn) {
		getProfileManager().getDeviceProfile(udn)->enabled() = true;
		notifyAlive(getProfileManager().getDeviceProfile(udn));
	}
	
	void UPnPServer::deactivateDevice(const UDN & udn) {
		getProfileManager().getDeviceProfile(udn)->enabled() = false;
		notifyByeBye(getProfileManager().getDeviceProfile(udn));
	}
	
	void UPnPServer::activateAllDevices() {
		vector< AutoRef<UPnPDeviceProfile> > profiles = getProfileManager().profiles();
		for (vector< AutoRef<UPnPDeviceProfile> >::iterator iter = profiles.begin();
			 iter != profiles.end(); iter++)
		{
			(*iter)->enabled() = true;
			delayNotify(0, NotifyTask::NOTIFY_ALIVE, (*iter));
			delayNotify(200, NotifyTask::NOTIFY_ALIVE, (*iter));
		}
	}
	
	void UPnPServer::deactivateAllDevices() {
		vector< AutoRef<UPnPDeviceProfile> > profiles = getProfileManager().profiles();
		for (vector< AutoRef<UPnPDeviceProfile> >::iterator iter = profiles.begin();
			 iter != profiles.end(); iter++)
		{
			(*iter)->enabled() = false;
			delayNotify(0, NotifyTask::NOTIFY_BYEBYE, (*iter));
			delayNotify(200, NotifyTask::NOTIFY_BYEBYE, (*iter));
		}
	}


	void UPnPServer::delayNotify(unsigned long delay, int type, AutoRef<UPnPDeviceProfile> profile) {
		timerThread.looper().delay(delay, AutoRef<TimerTask>(new NotifyTask(*this, type, profile)));
	}

	void UPnPServer::notifyAliveAll() {
		vector< AutoRef<UPnPDeviceProfile> > profiles = profileManager.profiles();
		for (vector< AutoRef<UPnPDeviceProfile> >::iterator iter = profiles.begin();
			 iter != profiles.end(); iter++)
		{
			if ((*iter)->enabled()) {
				notifyAlive((*iter));
			}
		}
	}
	
	void UPnPServer::notifyAlive(AutoRef<UPnPDeviceProfile> profile) {
		SSDPMsearchSender sender;
		string host = "239.255.255.250";
		int port = 1900;
		UDN udn = profile->udn();
		string location = makeLocation(udn);
		sender.sendMcastToAllInterfaces(makeNotifyAlive(location, udn, "upnp:rootdevice"),
										host, port);
		sender.sendMcastToAllInterfaces(makeNotifyAlive(location, udn, ""),
										host, port);
		vector<string> deviceTypes = profile->deviceTypes();
		for (vector<string>::iterator iter = deviceTypes.begin();
			 iter != deviceTypes.end(); iter++) {
			sender.sendMcastToAllInterfaces(makeNotifyAlive(location, udn, *iter),	host, port);
		}
		vector< AutoRef<UPnPService> > services = profile->allServices();
		for (vector< AutoRef<UPnPService> >::iterator iter = services.begin();
			 iter != services.end(); iter++) {
			sender.sendMcastToAllInterfaces(
				makeNotifyAlive(location, udn, (*iter)->serviceType()), host, port);
		}
		sender.close();
	}
	
	void UPnPServer::notifyAliveByDeviceType(AutoRef<UPnPDeviceProfile> profile, const string & deviceType) {

		UDN udn = profile->udn();
		string location = makeLocation(udn);
		
		SSDPMsearchSender sender;
		string packet = makeNotifyAlive(location, udn, deviceType);
		UPnPDebug::instance().debug("ssdp:notify-alive", packet);
		sender.sendMcastToAllInterfaces(packet, "239.255.255.250", 1900);
		sender.close();
	}
	
	string UPnPServer::makeNotifyAlive(const string & location, const UDN & udn, const string & deviceType) {

		USN usn(udn.toString());
		usn.rest() = deviceType;
		
		return "NOTIFY * HTTP/1.1\r\n"
			"Cache-Control: max-age=1800\r\n"
			"HOST: 239.255.255.250:1900\r\n"
			"Location: " + location + "\r\n"
			"NT: " + (deviceType.empty() ? usn.toString() : deviceType) + "\r\n"
			"NTS: ssdp:alive\r\n"
			"Server: " + config.getProperty("server.info", DEFAULT_SERVER_INFO) + "\r\n"
			"USN: " + usn.toString() + "\r\n"
			"\r\n";
	}

	void UPnPServer::notifyByeBye(AutoRef<UPnPDeviceProfile> profile) {

		SSDPMsearchSender sender;
		string host = "239.255.255.250";
		int port = 1900;

		UDN udn = profile->udn();
		string location = makeLocation(udn);
		sender.sendMcastToAllInterfaces(makeNotifyByeBye(udn, "upnp:rootdevice"), host, port);
		sender.sendMcastToAllInterfaces(makeNotifyByeBye(udn, ""), host, port);

		vector<string> deviceTypes = profile->deviceTypes();
		for (vector<string>::iterator iter = deviceTypes.begin(); iter != deviceTypes.end(); iter++) {
			sender.sendMcastToAllInterfaces(makeNotifyByeBye(udn, *iter), host, port);
		}

		vector< AutoRef<UPnPService> > services = profile->allServices();
		for (vector< AutoRef<UPnPService> >::iterator iter = services.begin();
			 iter != services.end(); iter++)
		{
			sender.sendMcastToAllInterfaces(
				makeNotifyByeBye(udn, (*iter)->serviceType()), host, port);
		}
		
		sender.close();
	}

	void UPnPServer::notifyByeByeByDeviceType(AutoRef<UPnPDeviceProfile> profile, const string & deviceType) {
		UDN udn = profile->udn();
		SSDPMsearchSender sender;
		string packet = makeNotifyByeBye(udn, deviceType);
		UPnPDebug::instance().debug("ssdp:notify-byebye", packet);
		sender.sendMcastToAllInterfaces(packet, "239.255.255.250", 1900);
		sender.close();
	}

	string UPnPServer::makeNotifyByeBye(const UDN & udn, const string & deviceType) {
		USN usn(udn.toString());
		usn.rest() = deviceType;
		return "NOTIFY * HTTP/1.1\r\n"
			"Host: 239.255.255.250:1900\r\n"
			"NT: " + (deviceType.empty() ? usn.toString() : deviceType)  + "\r\n"
			"NTS: ssdp:byebye\r\n"
			"USN: " + usn.toString() + "\r\n"
			"\r\n";
	}

	void UPnPServer::respondMsearch(const string & st, InetAddress & remoteAddr) {
		SSDPMsearchSender sender;

		vector<string> types = ((st == "ssdp:all") ?
								getProfileManager().getAllTypes() :
								getProfileManager().getTypes(st));

		for (vector<string>::iterator iter = types.begin(); iter != types.end(); iter++) {
			USN usn(*iter);
			UDN udn("uuid:" + usn.uuid());
			string location = makeLocation(udn);
			string packet = makeMsearchResponse(location, udn, usn.rest());
			UPnPDebug::instance().debug("ssdp:response-msearch", packet);
			sender.unicast(packet, remoteAddr);
		}
		sender.close();
	}

	string UPnPServer::makeMsearchResponse(const string & location, const UDN & udn, const string & st) {
		USN usn(udn.toString());
		return "HTTP/1.1 200 OK\r\n"
			"Cache-Control: max-age=1800\r\n"
			"HOST: 239.255.255.250:1900\r\n"
			"Location: " + location + "\r\n"
			"ST: " + (st.empty() ? usn.toString() : st) + "\r\n"
			"Server: " + config.getProperty("server.info", DEFAULT_SERVER_INFO) + "\r\n"
			"USN: " + usn.toString() + "\r\n"
			"Ext: \r\n"
			"Date: " + Date::formatRfc1123(Date::now()) + "\r\n"
			"\r\n";
	}

	UPnPDeviceProfileManager & UPnPServer::getProfileManager() {
		return profileManager;
	}

	AutoRef<UPnPDeviceProfile> UPnPServer::registerDeviceProfile(const Url & url) {
		UPnPDeviceBuilder builder(url);
		AutoRef<UPnPDeviceProfile> profile(new UPnPDeviceProfile(builder.execute()));
		registerDeviceProfile(profile);
		return profile;
	}
	
	void UPnPServer::registerDeviceProfile(AutoRef<UPnPDeviceProfile> profile) {
		getProfileManager().registerProfile(profile);
	}

	void UPnPServer::unregisterDeviceProfile(const UDN & udn) {
		getProfileManager().unregisterProfile(udn);
	}
	
	void UPnPServer::setActionRequestHandler(AutoRef<UPnPActionRequestHandler> actionRequestHandler) {
		this->actionRequestHandler = actionRequestHandler;
	}

	AutoRef<UPnPActionRequestHandler> UPnPServer::getActionRequestHandler() {
		return actionRequestHandler;
	}

	void UPnPServer::setHttpEventListener(AutoRef<HttpEventListener> httpEventListener) {
		this->httpEventListener = httpEventListener;
	}
	
	AutoRef<HttpEventListener> UPnPServer::getHttpEventListener() {
		return httpEventListener;
	}

	UPnPPropertyManager & UPnPServer::getPropertyManager() {
		return propertyManager;
	}

	void UPnPServer::setProperty(const UDN & udn, const string & serviceType,
								 const string & name, const string & value) {
		getPropertyManager().setProperty(udn, serviceType, name, value);
	}

	void UPnPServer::setProperties(const UDN & udn, const string & serviceType,
								   LinkedStringMap & props) {
		getPropertyManager().setProperties(udn, serviceType, props);
	}

	void UPnPServer::notifyEvent(const string & sid) {
		notificationThread.notify(sid);
	}
	
	void UPnPServer::delayNotifyEvent(const string & sid, unsigned long delay) {
		notificationThread.delayNotify(sid, delay);
	}


	string UPnPServer::onSubscribe(AutoRef<UPnPDevice> device,
								   AutoRef<UPnPService> service,
								   const vector<string> & callbacks,
								   unsigned long timeout) {
		
		UuidGeneratorVersion1 gen;
		string sid = Uuid(gen.generate()).toString();

		AutoRef<UPnPEventSubscriptionSession> session(new UPnPEventSubscriptionSession);
		session->sid() = sid;
		session->callbackUrls() = callbacks;
		session->updateTime();
		session->timeout() = timeout;
		session->udn() = device->udn();
		session->serviceType() = service->serviceType();

		propertyManager.addSubscriptionSession(session);

		return sid;
	}

	bool UPnPServer::onRenewSubscription(const string & sid, unsigned long timeout) {
		if (propertyManager.hasSubscriptionSession(sid)) {
			propertyManager.getSession(sid)->updateTime();
			propertyManager.getSession(sid)->timeout() = timeout;
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

	TimerLooperThread & UPnPServer::getTimerThread() {
		return timerThread;
	}

	void UPnPServer::collectExpired() {
		propertyManager.collectExpired();
	}

	string UPnPServer::getServerInfo() {
		return config.getProperty("server.info", DEFAULT_SERVER_INFO);
	}

}
