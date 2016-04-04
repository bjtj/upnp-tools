#include "UPnPSession.hpp"

#include "UPnPDeviceDeserializer.hpp"

namespace UPNP {

	using namespace std;
	using namespace UTIL;
	using namespace XML;
	
	UPnPSession::UPnPSession(const string & udn) :
		_udn(udn), _completed(false), creationTime(0), updateTime(0), sessionTimeout(0) {

		updateTime = creationTime = OS::tick_milli();
		sessionTimeout = DEFAULT_TIMEOUT;
	}
	UPnPSession::~UPnPSession() {
		printf("[%s] session instance destroyed\n", _udn.c_str());
	}

	string & UPnPSession::udn() {
		return _udn;
	}

	void UPnPSession::setCreationTime(unsigned long creationTime) {
		this->creationTime = creationTime;
	}

	void UPnPSession::setUpdateTime(unsigned long updateTime) {
		this->updateTime = updateTime;
	}

	void UPnPSession::setSessionTimeout(unsigned long sessionTimeout) {
		this->sessionTimeout = sessionTimeout;
	}

	unsigned long UPnPSession::lifetimeFromCreation() {
		return (OS::tick_milli() - creationTime);
	}

	unsigned long UPnPSession::lifetimeFromLastUpdate() {
		return (OS::tick_milli() - updateTime);
	}

	bool UPnPSession::outdated() {
		return (lifetimeFromLastUpdate() >= sessionTimeout);
	}

	void UPnPSession::prolong(const string & cacheControl) {
		unsigned long timeout = parseCacheControlMilli(cacheControl, DEFAULT_TIMEOUT);
		if (timeout < DEFAULT_TIMEOUT) {
			timeout = DEFAULT_TIMEOUT;
		}
		prolong(timeout);
	}
	void UPnPSession::prolong(unsigned long timeout) {
		this->updateTime = OS::tick_milli();
		this->sessionTimeout = timeout;
	}

	string UPnPSession::getDump(const HTTP::Url & url) {
		return HttpUtils::httpGet(url);
	}

	void UPnPSession::buildDevice(SSDP::SSDPHeader & header) {
		rootDevice = UPnPDeviceDeserializer::buildDevice(header);
		prolong(header.getCacheControl());
	}

	bool UPnPSession::completed() {
		return _completed;
	}

	AutoRef<UPnPDevice> UPnPSession::getRootDevice() {
		return rootDevice;
	}
	
	unsigned long UPnPSession::parseCacheControlMilli(const std::string & cacheControl, unsigned long def) {
		if (!Text::startsWith(cacheControl, "max-age=")) {
			return def;
		}
	}
}

