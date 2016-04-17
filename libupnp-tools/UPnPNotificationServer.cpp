#include "UPnPNotificationServer.hpp"
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
	class UPnPNotificationParser {
	private:
	public:
		UPnPNotificationParser() {}
		virtual ~UPnPNotificationParser() {}

		static map<string, string> parseNotify(const string & xml) {
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
	};

	/**
	 * @brief 
	 */
	class EventNotificationHandler : public HttpRequestHandler {
	private:
		UPnPNotificationListener * listener;
	public:
		EventNotificationHandler(UPnPNotificationListener * listener) : listener(listener) {}
		virtual ~EventNotificationHandler() {}
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
			map<string, string> props = UPnPNotificationParser::parseNotify(dump);
			for (map<string, string>::iterator iter = props.begin(); iter != props.end(); iter++) {
				notify[iter->first] = iter->second;
			}

			if (listener) {
				listener->onNotify(notify);
			}

			response.setStatusCode(200);
		}	
	};
	
	
	UPnPNotificationServer::UPnPNotificationServer(UPnPNotificationServerConfig & config) : config(config), server(NULL) {
	}
	UPnPNotificationServer::~UPnPNotificationServer() {
	}
	void UPnPNotificationServer::startAsync() {
		
		if (server) {
			return;
		}

		HttpServerConfig httpServerConfig;
		httpServerConfig["listen.port"] = config["listen.port"];
		httpServerConfig["thread.count"] = config.getProperty("thread.count", "5");

		server = new AnotherHttpServer(httpServerConfig);
		AutoRef<HttpRequestHandler> handler(new EventNotificationHandler(this));
		server->registerRequestHandler("/*", handler);
		server->startAsync();
	}
	void UPnPNotificationServer::stop() {
		if (!server) {
			return;
		}

		server->stop();
		delete server;
		server = NULL;
	}
	void UPnPNotificationServer::addSubscription(UPnPEventSubscription & subscription) {
		registry.addSubscription(subscription);
	}
	void UPnPNotificationServer::removeSubscription(UPnPEventSubscription & subscription) {
		registry.removeSubscription(subscription.sid());
	}
	void UPnPNotificationServer::addNotificationListener(AutoRef<UPnPNotificationListener> listener) {
		listeners.push_back(listener);
	}

	UPnPEventSubscription & UPnPNotificationServer::findSubscriptionWithUdnAndServiceType(const string & udn, const string & serviceType) {
		return registry.findSubscriptionWithUdnAndServiceType(udn, serviceType);
	}
	
	void UPnPNotificationServer::onNotify(UPnPNotify & notify) {

		if (!registry.hasSubscription(notify.sid())) {
			throw Exception("Not registred subscription ID : " + notify.sid());
		}

		notify.subscription() = registry[notify.sid()];
		
		for (vector<AutoRef<UPnPNotificationListener> >::iterator iter = listeners.begin(); iter != listeners.end(); iter++) {
			(*iter)->onNotify(notify);
		}
	}

	string UPnPNotificationServer::getCallbackUrl(const string & host) {
		return "http://" + host + ":" + config["listen.port"] + "/notify";
	}
}
