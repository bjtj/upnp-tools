#include <liboslayer/Object.hpp>
#include <liboslayer/Uuid.hpp>
#include <liboslayer/File.hpp>
#include <liboslayer/Logger.hpp>
#include "UPnPPropertyManager.hpp"
#include "HttpUtils.hpp"
#include "XmlUtils.hpp"

namespace upnp {

	using namespace std;
	using namespace osl;
    
    static AutoRef<Logger> logger = LoggerFactory::instance().getObservingLogger(File::basename(__FILE__));


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

	NotificationRequest::NotificationRequest()
		: delay(0)
	{
		creationTick = tick_milli();
	}
	
	NotificationRequest::NotificationRequest(const string & sid)
		: _sid(sid), delay(0)
	{
		creationTick = tick_milli();
	}
	
	NotificationRequest::NotificationRequest(const string & sid, unsigned long delay)
		: _sid(sid), delay(delay)
	{
		creationTick = tick_milli();
	}
	
	NotificationRequest::~NotificationRequest() {
	}
	
	string & NotificationRequest::sid() {
		return _sid;
	}
	
	bool NotificationRequest::prepared() {
		return (tick_milli() - creationTick) >= delay;
	}


	/**
	 * @brief
	 */
	UPnPEventNotificationThread::UPnPEventNotificationThread(UPnPPropertyManager & propertyManager)
		: propertyManager(propertyManager) {
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
				if (messageQueue.front().obj()->prepared()) {
					string sid = messageQueue.dequeue().obj()->sid();
					try {
						propertyManager.notify(sid);
					} catch (Exception e) {
						// TODO: handle error
					}
				}
			}
		}
	}
	
	void UPnPEventNotificationThread::notify(const string & sid) {
		messageQueue.enqueue(NotificationRequestMessage(0, AutoNotificationRequest(
															new NotificationRequest(sid))));
	}
	
	void UPnPEventNotificationThread::delayNotify(const string & sid, unsigned long delay) {
		messageQueue.enqueue(NotificationRequestMessage(0, AutoNotificationRequest(
															new NotificationRequest(sid, delay))));
	}

	/**
	 * @brief
	 */
	UPnPPropertyManager::UPnPPropertyManager() {
	}
	
	UPnPPropertyManager::~UPnPPropertyManager() {
	}
	
	string UPnPPropertyManager::getKey(const string & udn, const string serviceType) {
		return udn + "::" + serviceType;
	}
	
	void UPnPPropertyManager::clear() {
		registry.clear();
		sessions.clear();
	}
	
	bool UPnPPropertyManager::isRegisteredService(const string & udn, const string serviceType) {
		return registry.find(getKey(udn, serviceType)) != registry.end();
	}
	
	void UPnPPropertyManager::registerService(const string & udn, const std::string serviceType, const LinkedStringMap & props) {
		registry[getKey(udn, serviceType)] = props;
	}
	
	void UPnPPropertyManager::addSubscriptionSession(const AutoRef<UPnPEventSubscriptionSession> session) {
		sessions[session->sid()] = session;
	}
	
	bool UPnPPropertyManager::hasSubscriptionSession(const string & sid) {
		return (sessions.find(sid) != sessions.end());
	}
	
	void UPnPPropertyManager::removeSubscriptionSession(const string & sid) {
		sessions.erase(sid);
	}
	
	AutoRef<UPnPEventSubscriptionSession> UPnPPropertyManager::getSession(const string & sid) {
		if (hasSubscriptionSession(sid)) {
			return sessions[sid];
		}
		throw Exception("no session found with sid : " + sid);
	}
	
	vector< AutoRef<UPnPEventSubscriptionSession> > UPnPPropertyManager::getSessionsByUdnAndServiceType(const string & udn, const string & serviceType) {
		vector< AutoRef<UPnPEventSubscriptionSession> > ret;
		for (map< string, AutoRef<UPnPEventSubscriptionSession> >::iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
			if (iter->second->udn() == udn && iter->second->serviceType() == serviceType) {
				ret.push_back(iter->second);
			}
		}
		return ret;
	}

	void UPnPPropertyManager::setProperty(const string & udn, const string & serviceType,
										  const string & name, const string & value) {
		LinkedStringMap & props = registry[getKey(udn, serviceType)];
		props[name] = value;
		notify(getSessionsByUdnAndServiceType(udn, serviceType), props);
	}
	
	void UPnPPropertyManager::setProperties(const string & udn, const string & serviceType,
											const LinkedStringMap & props) {
		registry[getKey(udn, serviceType)] = props;
		notify(getSessionsByUdnAndServiceType(udn, serviceType), props);
	}

	LinkedStringMap & UPnPPropertyManager::getProperties(const string & udn, const string & serviceType) {
		return registry[getKey(udn, serviceType)];
	}
	
	LinkedStringMap & UPnPPropertyManager::getPropertiesBySid(const string & sid) {
		AutoRef<UPnPEventSubscriptionSession> session = getSession(sid);
		return registry[getKey(session->udn(), session->serviceType())];
	}

	void UPnPPropertyManager::notify(const string & sid) {
		notify(getSession(sid), getPropertiesBySid(sid));
	}

	void UPnPPropertyManager::notify(const vector< AutoRef<UPnPEventSubscriptionSession> > & sessions, const LinkedStringMap & props) {
		for (vector< AutoRef<UPnPEventSubscriptionSession> >::const_iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
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
            try {
                HttpUtils::httpPost("NOTIFY", http::Url(*iter), headers, content);
            } catch (Exception e) {
                logger->error("notify event failed with - " + e.message());
            }
		}
	}

	string UPnPPropertyManager::makePropertiesXml(const LinkedStringMap & props) {
		string content;
		content.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n");
		content.append("<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\">\r\n");
		content.append("<e:property>\r\n");
		for (size_t i = 0; i < props.size(); i++) {
			content.append(XmlUtils::toKeyValueTag(props[i]));
			content.append("\r\n");
		}
		content.append("</e:property>\r\n");
		content.append("</e:propertyset>\r\n");
		return content;
	}

	void UPnPPropertyManager::collectExpired() {
		for (map< string, AutoRef<UPnPEventSubscriptionSession> >::iterator iter = sessions.begin(); iter != sessions.end();) {
			if (iter->second->expired()) {
				if (!outdatedListener.nil()) {
					outdatedListener->onSessionOutdated(*(iter->second));
				}
				sessions.erase(iter++);
			} else {
				iter++;
			}
		}
	}

	void UPnPPropertyManager::setOnSubscriptionOutdatedListener(AutoRef<OnSubscriptionOutdatedListener> listener) {
		outdatedListener = listener;
	}
}
