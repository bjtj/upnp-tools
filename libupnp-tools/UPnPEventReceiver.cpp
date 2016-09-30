#include "UPnPEventReceiver.hpp"
#include <libhttp-server/AnotherHttpServer.hpp>
#include <libhttp-server/StringDataSink.hpp>
#include <liboslayer/Text.hpp>
#include <liboslayer/XmlParser.hpp>
#include "XmlUtils.hpp"

namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace UTIL;
	using namespace HTTP;
	using namespace XML;

	/**
	 * @brief 
	 */

	UPnPEventSubscriptionRegistry::UPnPEventSubscriptionRegistry() {
	}
	
	UPnPEventSubscriptionRegistry::~UPnPEventSubscriptionRegistry() {
	}
	
	void UPnPEventSubscriptionRegistry::clear() {
		subscriptions.clear();
	}
	
	void UPnPEventSubscriptionRegistry::addSubscription(UPnPEventSubscription & subscription) {
		subscriptions[subscription.sid()] = subscription;
	}
	
	void UPnPEventSubscriptionRegistry::removeSubscription(const string & sid) {
		subscriptions.erase(sid);
	}
	
	bool UPnPEventSubscriptionRegistry::hasSubscription(const string & sid) {
		return subscriptions.find(sid) != subscriptions.end();
	}
	
	UPnPEventSubscription & UPnPEventSubscriptionRegistry::findSubscriptionByUdnAndServiceType(const string & udn, const string & serviceType) {
		for (map<string, UPnPEventSubscription>::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++) {
			if (iter->second.udn() == udn && iter->second.serviceType() == serviceType) {
				return iter->second;
			}
		}
		throw Exception("No subscription found");
	}
	
	UPnPEventSubscription & UPnPEventSubscriptionRegistry::operator[] (const string & sid) {
		return subscriptions[sid];
	}
	

	/**
	 * @brief 
	 */
	class EventNotificationHandler : public HttpRequestHandler {
	private:
		UPnPEventReceiver & receiver;
	public:
		EventNotificationHandler(UPnPEventReceiver & receiver) : receiver(receiver) {
		}
		virtual ~EventNotificationHandler() {
		}
		virtual AutoRef<DataSink> getDataSink() {
			return AutoRef<DataSink>(new StringDataSink);
		}
		virtual void onHttpRequestContentCompleted(HttpRequest & request, AutoRef<DataSink> sink, HttpResponse & response) {
			
			if (sink.nil()) {
				throw Exception("wrong event notify / no content");
			}
			
			string sid = request.getHeader()["SID"];
			string seq = request.getHeader()["SEQ"];
			UPnPNotify notify(sid, Text::toLong(seq));
			string dump = ((StringDataSink*)&sink)->data();
			map<string, string> props = receiver.parseEventNotify(dump);
			for (map<string, string>::iterator iter = props.begin(); iter != props.end(); iter++) {
				notify[iter->first] = iter->second;
			}

			receiver.onNotify(notify);

			response.setStatus(200);
		}	
	};
	

	/**
	 * @brief 
	 */
	
	UPnPEventReceiver::UPnPEventReceiver(UPnPEventReceiverConfig & config) : config(config), server(NULL) {
	}
	
	UPnPEventReceiver::~UPnPEventReceiver() {
	}
	
	void UPnPEventReceiver::startAsync() {
		
		if (server) {
			return;
		}

		HttpServerConfig httpServerConfig;
		httpServerConfig["listen.port"] = config["listen.port"];
		httpServerConfig["thread.count"] = config.getProperty("thread.count", "5");

		server = new AnotherHttpServer(httpServerConfig);
		AutoRef<HttpRequestHandler> handler(new EventNotificationHandler(*this));
		server->registerRequestHandler("/*", handler);
		server->startAsync();
	}
	
	void UPnPEventReceiver::stop() {
		
		if (!server) {
			return;
		}

		server->stop();
		delete server;
		server = NULL;
	}
	
	void UPnPEventReceiver::addSubscription(UPnPEventSubscription & subscription) {
		registry.addSubscription(subscription);
	}
	
	void UPnPEventReceiver::removeSubscription(UPnPEventSubscription & subscription) {
		registry.removeSubscription(subscription.sid());
	}
	
	void UPnPEventReceiver::addEventListener(AutoRef<UPnPEventListener> listener) {
		listeners.push_back(listener);
	}

	UPnPEventSubscription & UPnPEventReceiver::findSubscriptionByUdnAndServiceType(const string & udn, const string & serviceType) {
		return registry.findSubscriptionByUdnAndServiceType(udn, serviceType);
	}
	
	void UPnPEventReceiver::onNotify(UPnPNotify & notify) {

		if (!registry.hasSubscription(notify.sid())) {
			throw Exception("No registred subscription ID : " + notify.sid());
		}

		notify.subscription() = registry[notify.sid()];
		
		for (vector<AutoRef<UPnPEventListener> >::iterator iter = listeners.begin(); iter != listeners.end(); iter++) {
			(*iter)->onNotify(notify);
		}
	}

	string UPnPEventReceiver::getCallbackUrl(const string & host) {
		return "http://" + host + ":" + config["listen.port"] + "/notify";
	}


	map<string, string> UPnPEventReceiver::parseEventNotify(const string & xml) {
		map<string, string> props;
		XmlDocument doc = DomParser::parse(xml);
		if (doc.getRootNode().nil()) {
			return props;
		}
		vector<XmlNode*> nodes = doc.getRootNode()->getElementsByTagName("property");
		for (vector<XmlNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++) {
			XmlNode * node = *iter;
			NameValue nv = XmlUtils::toNameValue(node->getFirstChildElement());
			props[nv.name()] = nv.value();
		}
		return props;
	}
}
