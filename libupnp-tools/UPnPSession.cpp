#include "UPnPSession.hpp"

#include "UPnPDeviceDeserializer.hpp"

namespace UPNP {

	using namespace std;
	using namespace UTIL;
	using namespace XML;
	
	UPnPSession::UPnPSession(const string & udn) :
		udn(udn), _completed(false), creationTime(0), updateTime(0), sessionTimeout(0) {
	}
	UPnPSession::~UPnPSession() {
		printf("[%s] session instance destroyed\n", udn.c_str());
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

	unsigned long UPnPSession::lifetime() {
		return (OS::tick_milli() - creationTime);
	}

	unsigned long UPnPSession::duration() {
		return (OS::tick_milli() - updateTime);
	}

	bool UPnPSession::outdated() {
		return (duration() >= sessionTimeout);
	}

	string UPnPSession::getDump(const HTTP::Url & url) {
		return HttpUtils::httpGet(url);
	}

	void UPnPSession::buildDevice(SSDP::SSDPHeader & header) {
		rootDevice = UPnPDeviceDeserializer::buildDevice(header);
	}

	bool UPnPSession::completed() {
		return _completed;
	}

	AutoRef<UPnPDevice> UPnPSession::getRootDevice() {
		return rootDevice;
	}

	string UPnPSession::toString() {
		if (rootDevice.nil()) {
			return "";
		}
		return toString(*rootDevice, 0);
	}

	string UPnPSession::toString(UPnPDevice & device, int depth) {
		string str;

		str.append(depth, ' ');
		if (depth > 0) { str.append(" - "); }
		str.append(device.getUdn() + " (" + device.getFriendlyName() + ")");

		vector<AutoRef<UPnPService> > services = device.services();
		for (vector<AutoRef<UPnPService> >::iterator iter = services.begin(); iter != services.end(); iter++) {
			str.append("\n");
			str.append(depth, ' ');
			str.append(" ** " + (*iter)->getServiceType());

			vector<UPnPAction> actions = (*iter)->actions();
			for (vector<UPnPAction>::iterator aiter = actions.begin(); aiter != actions.end(); aiter++) {
				str.append("\n");
				str.append(depth, ' ');
				str.append("  - " + (*aiter).name());
			}
		}
			
		vector<AutoRef<UPnPDevice> > & devices = device.devices();
		for (vector<AutoRef<UPnPDevice> >::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			str.append("\n");
			str.append(toString(*(*iter), depth + 1));
		}
		return str;
	}
	
}

