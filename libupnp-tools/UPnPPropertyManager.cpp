#include <liboslayer/Object.hpp>
#include <liboslayer/Uuid.hpp>
#include "UPnPPropertyManager.hpp"
#include "HttpUtils.hpp"
#include "XmlUtils.hpp"

namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace UTIL;


	UPnPEventSubscriptionSession::UPnPEventSubscriptionSession() {
	}
	UPnPEventSubscriptionSession::~UPnPEventSubscriptionSession() {
	}
	vector<string> & UPnPEventSubscriptionSession::callbackUrls() {
		return _callbackUrls;
	}

	/**
	 * @brief
	 */
	class NotificationRequestObject : public Object {
	private:
		unsigned long creationTick;
		string _sid;
		unsigned long delay;
	public:
		NotificationRequestObject() : delay(0) {
			creationTick = tick_milli();
		}
		NotificationRequestObject(const string & sid) :  _sid(sid), delay(0) {
			creationTick = tick_milli();
		}
		NotificationRequestObject(const string & sid, unsigned long delay) : _sid(sid), delay(delay) {
			creationTick = tick_milli();
		}
		virtual ~NotificationRequestObject() {
		}
		string & sid() {
			return _sid;
		}
		bool prepared() {
			return (tick_milli() - creationTick) >= delay;
		}
		static bool prepared(Message msg) {
			return ((NotificationRequestObject*)&msg.obj())->prepared();
		}
		static string sid(Message msg) {
			return ((NotificationRequestObject*)&msg.obj())->sid();
		}
	};

	/**
	 * @brief
	 */
	UPnPEventNotificationThread::UPnPEventNotificationThread(UPnPPropertyManager & propertyManager) : propertyManager(propertyManager) {
	}
	UPnPEventNotificationThread::~UPnPEventNotificationThread() {
	}
	void UPnPEventNotificationThread::run() {
		while (!interrupted()) {
			if (messageQueue.size() == 0) {
				idle(10);
				continue;
			}
			size_t size = messageQueue.size();
			while (size-- > 0) {
				if (NotificationRequestObject::prepared(messageQueue.front())) {
					string sid = NotificationRequestObject::sid(messageQueue.dequeue());
					propertyManager.notify(sid);
				}
			}
		}
	}
	void UPnPEventNotificationThread::notify(const string & sid) {
		messageQueue.enqueue(Message(0, AutoRef<Object>(new NotificationRequestObject(sid))));
	}
	void UPnPEventNotificationThread::delayNotify(const string & sid, unsigned long delay) {
		messageQueue.enqueue(Message(0, AutoRef<Object>(new NotificationRequestObject(sid, delay))));
	}

	/**
	 * @brief
	 */
	UPnPPropertyManager::UPnPPropertyManager() {
	}
	UPnPPropertyManager::~UPnPPropertyManager() {
	}
	string UPnPPropertyManager::makeKey(const string & udn, const string serviceType) {
		Uuid uuid(udn);
		return uuid.getUuid() + "::" + serviceType;
	}
	void UPnPPropertyManager::clear() {
		registry.clear();
		sessions.clear();
	}
	bool UPnPPropertyManager::isRegisteredService(const string & udn, const string serviceType) {
		return registry.find(makeKey(udn, serviceType)) != registry.end();
	}
	void UPnPPropertyManager::registerService(const std::string & udn, const std::string serviceType, const LinkedStringMap & props) {
		registry[makeKey(udn, serviceType)] = props;
	}
	void UPnPPropertyManager::addSubscriptionSession(const AutoRef<UPnPEventSubscriptionSession> session) {
		sessions[session->sid()] = session;
	}
	void UPnPPropertyManager::removeSubscriptionSession(const string & sid) {
		sessions.erase(sid);
	}
	AutoRef<UPnPEventSubscriptionSession> UPnPPropertyManager::getSession(const string & sid) {
		return sessions[sid];
	}
	
	vector<AutoRef<UPnPEventSubscriptionSession> > UPnPPropertyManager::getSessionsByUdnAndServiceType(const string & udn, const string & serviceType) {
		vector<AutoRef<UPnPEventSubscriptionSession> > ret;
		for (map<string, AutoRef<UPnPEventSubscriptionSession> >::iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
			if (iter->second->udn() == udn && iter->second->serviceType() == serviceType) {
				ret.push_back(iter->second);
			}
		}
		return ret;
	}
	
	void UPnPPropertyManager::setProperties(const string & udn, const string & serviceType, const LinkedStringMap & props) {
		registry[makeKey(udn, serviceType)] = props;
		notify(getSessionsByUdnAndServiceType(udn, serviceType), props);
	}

	LinkedStringMap & UPnPPropertyManager::getProperties(const string & udn, const string & serviceType) {
		return registry[makeKey(udn, serviceType)];
	}
	
	LinkedStringMap & UPnPPropertyManager::getPropertiesBySid(const string & sid) {
		AutoRef<UPnPEventSubscriptionSession> session = getSession(sid);
		return registry[makeKey(session->udn(), session->serviceType())];
	}

	void UPnPPropertyManager::notify(const string & sid) {
		notify(getSession(sid), getPropertiesBySid(sid));
	}

	void UPnPPropertyManager::notify(const vector<AutoRef<UPnPEventSubscriptionSession> > & sessions, const LinkedStringMap & props) {
		for (vector<AutoRef<UPnPEventSubscriptionSession> >::const_iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
			notify(*iter, props);
		}
	}

	void UPnPPropertyManager::notify(AutoRef<UPnPEventSubscriptionSession> session, const LinkedStringMap & props) {
		vector<string> urls = session->callbackUrls();
		LinkedStringMap headers;
		headers["SID"] = session->sid();
		headers["SEQ"] = Text::toString(session->lastSeq()++);
		headers["Content-Type"] = "text/xml";
		string content = makePropertiesXml(props);
		for (vector<string>::iterator iter = urls.begin(); iter != urls.end(); iter++) {
			HttpUtils::httpPost("NOTIFY", HTTP::Url(*iter), headers, content);
		}
	}

	

	string UPnPPropertyManager::makePropertiesXml(const LinkedStringMap & props) {
		string content;
		content.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n");
		content.append("<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\">\r\n");
		content.append("<e:property>\r\n");
		for (size_t i = 0; i < props.size(); i++) {
			content.append(XmlUtils::toNameValueTag(props[i]));
			content.append("\r\n");
		}
		content.append("</e:property>\r\n");
		content.append("</e:propertyset>\r\n");
		return content;
	}

	void UPnPPropertyManager::collectOutdated() {
		for (map<string, AutoRef<UPnPEventSubscriptionSession> >::iterator iter = sessions.begin(); iter != sessions.end();) {
			if (iter->second->outdated()) {
				sessions.erase(iter++);
			} else {
				iter++;
			}
		}
	}
}
