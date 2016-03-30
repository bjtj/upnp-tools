#include <iostream>
#include <liboslayer/XmlParser.hpp>
#include <liboslayer/MessageQueue.hpp>
#include "UPnPServer.hpp"
#include "SSDPMsearchSender.hpp"
#include "NetworkUtil.hpp"
#include "UPnPActionRequest.hpp"
#include "UPnPActionResponse.hpp"
#include "XmlUtils.hpp"
#include "Uuid.hpp"

namespace UPNP {

	using namespace std;
	using namespace SSDP;
	using namespace HTTP;
	using namespace UTIL;

	/**
	 * @breif life time task
	 */
	class LifetimeTask : public TimerTask {
	private:
	public:
		LifetimeTask() {}
		virtual ~LifetimeTask() {}
		virtual void doTask() {
			// TODO: notify alive profiles
			// TODO: check outdated resources
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

		virtual void onHttpResponseTransferCompleted(HttpRequest & request, HttpResponse & response) {
			//
		}

		virtual void onHttpRequestContentCompleted(HttpRequest & request, HttpResponse & response) {
			
			string uri = request.getHeader().getPart2();

			if (request.getPath() == "/device.xml") {
				response.setStatusCode(200);
				response.setContentType("text/xml");
				UPnPDeviceProfile profile = server.getDeviceProfileWithUdn(request.getParameter("udn"));
				setFixedTransfer(response, profile.deviceDescription());
				return;
			}
			
			if (server.hasDeviceProfileWithScpdUrl(uri)) {
				UPnPDeviceProfile deviceProfile = server.getDeviceProfileHasScpdUrl(uri);
				response.setStatusCode(200);
				response.setContentType("text/xml");
				UPnPServiceProfile serviceProfile = deviceProfile.getServiceProfileWithScpdUrl(uri);
				setFixedTransfer(response, serviceProfile.scpd());
				return;
			}
			
			if (server.hasDeviceProfileWithControlUrl(uri)) {
				
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

			if (server.hasDeviceProfileWithEventSubUrl(uri)) {
				
				// event sub/unsub

				UPnPDeviceProfile deviceProfile = server.getDeviceProfileHasEventSubUrl(uri);
				UPnPServiceProfile serviceProfile = deviceProfile.getServiceProfileWithEventSubUrl(uri);
				UPnPNotificationCenter & nc = server.getNotificationCenter();

				if (request.getMethod() == "SUBSCRIBE") {
					string callbackUrls = request.getHeaderField("CALLBACK");
					cout << " ** subscribe / callback urls : " << callbackUrls << endl;
					vector<string> urls = parseCallbackUrls(callbackUrls);
					string timeout = request.getParameter("TIMEOUT");
					unsigned long timeoutMilli = parseTimeout(timeout);

					UuidGeneratorDefault gen;
					string sid = gen.generate();

					UPnPEventSubscriptionSession session;
					session.sid() = sid;
					session.callbackUrls() = urls;
					session.setTimeout(timeoutMilli);
					session.udn() = deviceProfile.udn();
					session.serviceType() = serviceProfile.serviceType();

					nc.addSubscriptionSession(session);

					server.getEventNotifyThread().delayNotify(100, sid);

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
			if (!server.getActionHandler().nil()) {
				server.getActionHandler()->handleActionRequest(request, response);
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

			string xml = request.getTransfer()->getString();
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
	UPnPServer::UPnPServer(UPnPServerConfig & config) : httpServer(NULL), config(config), notifyThread(notificationCenter) {
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

		notifyThread.start();
		
		timerThread.start();
		timerThread.looper().interval(10 * 1000, AutoRef<TimerTask>(new LifetimeTask));
	}
	void UPnPServer::stop() {
		
		if (!httpServer) {
			return;
		}

		timerThread.stop();
		timerThread.join();
		
		notifyThread.interrupt();
		notifyThread.join();
		
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
		url.setParameter("udn", deviceProfile.udn());
		return url.toString();
	}
		
	void UPnPServer::notifyAliveWithDeviceType(UPnPDeviceProfile & profile, const string & deviceType) {

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

	bool UPnPServer::hasDeviceProfileWithControlUrl(const std::string & controlUrl) {
		for (map<string, UPnPDeviceProfile>::iterator iter = deviceProfiles.begin(); iter != deviceProfiles.end(); iter++) {
			if (iter->second.hasServiceWithControlUrl(controlUrl)) {
				return true;
			}
		}
		return false;
	}
	bool UPnPServer::hasDeviceProfileWithEventSubUrl(const std::string & eventSubUrl) {
		for (map<string, UPnPDeviceProfile>::iterator iter = deviceProfiles.begin(); iter != deviceProfiles.end(); iter++) {
			if (iter->second.hasServiceWithEventSubUrl(eventSubUrl)) {
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

	UPnPDeviceProfile & UPnPServer::getDeviceProfileHasEventSubUrl(const std::string & eventSubUrl) {
		for (map<string, UPnPDeviceProfile>::iterator iter = deviceProfiles.begin(); iter != deviceProfiles.end(); iter++) {
			if (iter->second.hasServiceWithEventSubUrl(eventSubUrl)) {
				return iter->second;
			}
		}
		throw OS::Exception("not found deivce profile", -1, 0);
	}
	
	UPnPDeviceProfile & UPnPServer::operator[] (const string & udn) {
		return deviceProfiles[udn];
	}
	
	void UPnPServer::setActionHandler(AutoRef<UPnPActionHandler> actionHandler) {
		this->actionHandler = actionHandler;
	}

	AutoRef<UPnPActionHandler> UPnPServer::getActionHandler() {
		return actionHandler;
	}

	UPnPNotificationCenter & UPnPServer::getNotificationCenter() {
		return notificationCenter;
	}

	UPnPEventNotifyThread & UPnPServer::getEventNotifyThread() {
		return notifyThread;
	}

	TimerLooperThread & UPnPServer::getTimerThread() {
		return timerThread;
	}
}
