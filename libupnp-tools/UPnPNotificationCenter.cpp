#include "UPnPNotificationCenter.hpp"
#include "HttpUtils.hpp"
#include "XmlUtils.hpp"
#include <liboslayer/Object.hpp>

namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace UTIL;

	/**
	 * @brief
	 */
	class NotifyRequestObject : public Object {
	private:
		unsigned long creationTick;
		unsigned long delay;
		string _sid;
	public:
		NotifyRequestObject() : delay(0) {
			creationTick = tick_milli();
		}
		NotifyRequestObject(const string & sid) : delay(0), _sid(sid) {
			creationTick = tick_milli();
		}
		NotifyRequestObject(unsigned long delay, const string & sid) : delay(delay), _sid(sid) {
			creationTick = tick_milli();
		}
		virtual ~NotifyRequestObject() {
		}
		string & sid() {
			return _sid;
		}
		bool prepared() {
			return (tick_milli() - creationTick) >= delay;
		}
		static bool prepared(Message msg) {
			return ((NotifyRequestObject*)&msg.obj())->prepared();
		}
		static string sid(Message msg) {
			return ((NotifyRequestObject*)&msg.obj())->sid();
		}
	};

	/**
	 * @brief
	 */
	UPnPEventNotifyThread::UPnPEventNotifyThread(UPnPNotificationCenter & notificationCenter) : notificationCenter(notificationCenter) {
	}
	UPnPEventNotifyThread::~UPnPEventNotifyThread() {
	}
	void UPnPEventNotifyThread::run() {
		while (!interrupted()) {
			if (messageQueue.size() == 0) {
				idle(10);
				continue;
			}
			size_t size = messageQueue.size();
			while (size-- > 0) {
				if (NotifyRequestObject::prepared(messageQueue.front())) {
					string sid = NotifyRequestObject::sid(messageQueue.dequeue());
					notificationCenter.notify(sid);
				}
			}
		}
	}
	void UPnPEventNotifyThread::scheduleNotify(const string & sid) {
		messageQueue.enqueue(Message(0, AutoRef<Object>(new NotifyRequestObject(sid))));
	}
	void UPnPEventNotifyThread::delayNotify(unsigned long delay, const string & sid) {
		messageQueue.enqueue(Message(0, AutoRef<Object>(new NotifyRequestObject(delay, sid))));
	}

	/**
	 * @brief
	 */
	UPnPNotificationCenter::UPnPNotificationCenter() {
	}
	UPnPNotificationCenter::~UPnPNotificationCenter() {
	}
	void UPnPNotificationCenter::clear() {
		registry.clear();
		sessions.clear();
	}
	void UPnPNotificationCenter::registerService(const std::string & udn, const std::string serviceType, LinkedStringMap & props) {
		string key = udn + "::" + serviceType;
		registry[key] = props;
	}
	void UPnPNotificationCenter::addSubscriptionSession(UPnPEventSubscriptionSession & session) {
		sessions[session.sid()] = session;
	}
	void UPnPNotificationCenter::removeSubscriptionSession(const string & sid) {
		sessions.erase(sid);
	}
	UPnPEventSubscriptionSession & UPnPNotificationCenter::getSession(const string & sid) {
		return sessions[sid];
	}
	UPnPEventSubscriptionSession & UPnPNotificationCenter::getSessionWithUdnAndServiceType(const string & udn, const string & serviceType) {
		for (map<string, UPnPEventSubscriptionSession>::iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
			if (iter->second.udn() == udn && iter->second.serviceType() == serviceType) {
				return iter->second;
			}
		}
		throw OS::Exception("no sessions found");
	}
	void UPnPNotificationCenter::setProperties(const string & udn, const string & serviceType, LinkedStringMap & props) {
		string key = udn + "::" + serviceType;
		registry[key] = props;
		
		notify(getSessionWithUdnAndServiceType(udn, serviceType), props);
	}

	LinkedStringMap & UPnPNotificationCenter::getProperties(const string & udn, const string & serviceType) {
		string key = udn + "::" + serviceType;
		return registry[key];
	}
	LinkedStringMap & UPnPNotificationCenter::getPropertiesWithSid(const string & sid) {
		UPnPEventSubscriptionSession & session = getSession(sid);
		string key = session.udn() + "::" + session.serviceType();
		return registry[key];
	}

	void UPnPNotificationCenter::notify(const string & sid) {
		notify(getSession(sid), getPropertiesWithSid(sid));
	}

	void UPnPNotificationCenter::notify(UPnPEventSubscriptionSession & session, LinkedStringMap & props) {
		vector<string> urls = session.callbackUrls();
		LinkedStringMap headers;
		headers["SID"] = session.sid();
		headers["SEQ"] = Text::toString(session.lastSeq()++);
		headers["Content-Type"] = "text/xml";
		string content = makePropertiesXml(props);
		for (vector<string>::iterator iter = urls.begin(); iter != urls.end(); iter++) {
			HttpUtils::httpPost("NOTIFY", HTTP::Url(*iter), headers, content);
		}
	}

	string UPnPNotificationCenter::makePropertiesXml(LinkedStringMap & props) {
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
}
