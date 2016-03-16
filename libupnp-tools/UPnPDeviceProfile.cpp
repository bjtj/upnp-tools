#include "UPnPDeviceProfile.hpp"

namespace UPNP {

	using namespace std;

	UPnPDeviceProfile::UPnPDeviceProfile() {
	}
	UPnPDeviceProfile::~UPnPDeviceProfile() {
	}
	string & UPnPDeviceProfile::udn() {
		return _udn;
	}
	string & UPnPDeviceProfile::alias() {
		return _alias;
	}
	string & UPnPDeviceProfile::deviceDescription() {
		return _deviceDescription;
	}
	string & UPnPDeviceProfile::scpd(const string & serviceType) {
		return _scpds[serviceType];
	}
	bool UPnPDeviceProfile::hasServiceWithServiceType(const string & serviceType) {
		for (vector<UPnPService>::iterator iter = _services.begin(); iter != _services.end(); iter++) {
			if (iter->getServiceType() == serviceType) {
				return true;
			}
		}
		return false;
	}
	bool UPnPDeviceProfile::hasServiceWithScpdUrl(const string & scpdUrl) {
		for (vector<UPnPService>::iterator iter = _services.begin(); iter != _services.end(); iter++) {
			if (iter->getScpdUrl() == scpdUrl) {
				return true;
			}
		}
		return false;
	}
	bool UPnPDeviceProfile::hasServiceWithControlUrl(const string & controlUrl) {
		for (vector<UPnPService>::iterator iter = _services.begin(); iter != _services.end(); iter++) {
			if (iter->getControlUrl() == controlUrl) {
				return true;
			}
		}
		return false;
	}
	bool UPnPDeviceProfile::hasServiceWithEventSubUrl(const string & eventSubUrl) {
		for (vector<UPnPService>::iterator iter = _services.begin(); iter != _services.end(); iter++) {
			if (iter->getEventSubUrl() == eventSubUrl) {
				return true;
			}
		}
		return false;
	}
	UPnPService & UPnPDeviceProfile::getServiceWithServiceType(const string & serviceType) {
		for (vector<UPnPService>::iterator iter = _services.begin(); iter != _services.end(); iter++) {
			if (iter->getServiceType() == serviceType) {
				return *iter;
			}
		}
		throw OS::Exception("not found service", -1, 0);
	}
	UPnPService & UPnPDeviceProfile::getServiceWithScpdUrl(const string & scpdUrl) {
		for (vector<UPnPService>::iterator iter = _services.begin(); iter != _services.end(); iter++) {
			if (iter->getScpdUrl() == scpdUrl) {
				return *iter;
			}
		}
		throw OS::Exception("not found service", -1, 0);
	}
	UPnPService & UPnPDeviceProfile::getServiceWithControlUrl(const string & controlUrl) {
		for (vector<UPnPService>::iterator iter = _services.begin(); iter != _services.end(); iter++) {
			if (iter->getControlUrl() == controlUrl) {
				return *iter;
			}
		}
		throw OS::Exception("not found service", -1, 0);
	}
	UPnPService & UPnPDeviceProfile::getServiceWithEventSubUrl(const string & eventSubUrl) {
		for (vector<UPnPService>::iterator iter = _services.begin(); iter != _services.end(); iter++) {
			if (iter->getEventSubUrl() == eventSubUrl) {
				return *iter;
			}
		}
		throw OS::Exception("not found service", -1, 0);
	}
}
