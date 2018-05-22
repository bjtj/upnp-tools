#include "UPnPEventReceiver.hpp"
#include <libhttp-server/AnotherHttpServer.hpp>
#include <libhttp-server/StringDataSink.hpp>
#include <liboslayer/Text.hpp>
#include <liboslayer/XmlParser.hpp>
#include "XmlUtils.hpp"
#include "UPnPDebug.hpp"

namespace upnp {

	using namespace std;
	using namespace osl;
	using namespace http;


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
			
			string sid = request.header()["SID"];
			string seq = request.header()["SEQ"];
			UPnPPropertySet propset(sid, Text::toLong(seq));
			string dump = ((StringDataSink*)&sink)->data();
			UPnPDebug::instance().debug("upnp:event", request.header().toString() + dump);
			map<string, string> props = receiver.parsePropertySet(dump);
			for (map<string, string>::iterator iter = props.begin(); iter != props.end(); iter++) {
				propset[iter->first] = iter->second;
			}

			receiver.onNotify(propset);

			response.setStatus(200);
		}	
	};
	

	/**
	 * @brief 
	 */
	
	UPnPEventReceiver::UPnPEventReceiver(UPnPEventReceiverConfig & config) : config(config) {
	}
	
	UPnPEventReceiver::~UPnPEventReceiver() {
	}
	
	void UPnPEventReceiver::startAsync() {
		
		if (server.nil() == false) {
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
		
		if (server.nil() == true) {
			return;
		}

		server->stop();
		server = NULL;
	}

	map< string, UPnPEventSubscription > UPnPEventReceiver::getSubscriptions() {
		return subscriptions;
	}
	
	void UPnPEventReceiver::addSubscription(UPnPEventSubscription & subscription) {
		subscriptions[subscription.sid()] = subscription;
	}
	
	void UPnPEventReceiver::removeSubscription(UPnPEventSubscription & subscription) {
		subscriptions.erase(subscription.sid());
	}
	
	void UPnPEventReceiver::addEventListener(AutoRef<UPnPEventListener> listener) {
		listeners.push_back(listener);
	}

	UPnPEventSubscription & UPnPEventReceiver::findSubscriptionByUdnAndServiceType(const string & udn, const string & serviceType) {
		
		for (map<string, UPnPEventSubscription>::iterator iter = subscriptions.begin();
			 iter != subscriptions.end(); iter++)
		{
			if (iter->second.udn() == udn && iter->second.serviceType() == serviceType) {
				return iter->second;
			}
		}
		throw Exception("No subscription found");
	}
	
	void UPnPEventReceiver::onNotify(UPnPPropertySet & propset) {

		if (subscriptions.find(propset.sid()) == subscriptions.end()) {
			throw Exception("No registred subscription ID : " + propset.sid());
		}

		propset.subscription() = subscriptions[propset.sid()];
		
		for (vector< AutoRef<UPnPEventListener> >::iterator iter = listeners.begin();
			 iter != listeners.end(); iter++)
		{
			(*iter)->onNotify(propset);
		}
	}

	string UPnPEventReceiver::getCallbackUrl(const string & host) {
		return "http://" + host + ":" + config["listen.port"] + "/event";
	}

	map<string, string> UPnPEventReceiver::parsePropertySet(const string & xml) {
		map<string, string> props;
		XmlDocument doc = DomParser::parse(xml);
		if (doc.rootNode().nil()) {
			return props;
		}
		vector< AutoRef<XmlNode> > nodes = doc.rootNode()->getElementsByTagName("property");
		for (vector< AutoRef<XmlNode> >::iterator iter = nodes.begin();
			 iter != nodes.end(); iter++)
		{
			AutoRef<XmlNode> node = *iter;
			KeyValue kv = XmlUtils::toKeyValue(node->getFirstChildElement());
			props[kv.key()] = kv.value();
		}
		return props;
	}
}
