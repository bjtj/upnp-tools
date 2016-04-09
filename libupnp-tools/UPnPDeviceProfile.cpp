#include "UPnPDeviceProfile.hpp"

namespace UPNP {

	using namespace std;

	UPnPDeviceProfile::UPnPDeviceProfile() {
	}
	UPnPDeviceProfile::~UPnPDeviceProfile() {
	}
	string & UPnPDeviceProfile::uuid() {
		return _uuid;
	}
	string & UPnPDeviceProfile::deviceDescription() {
		return _deviceDescription;
	}
	vector<string> & UPnPDeviceProfile::deviceTypes() {
		return _deviceTypes;
	}
	string UPnPDeviceProfile::rootDeviceType() {
		return _deviceTypes[0];
	}
	vector<UPnPServiceProfile> & UPnPDeviceProfile::serviceProfiles() {
		return _serviceProfiles;
	}
	string & UPnPDeviceProfile::scpd(const string & serviceType) {
		return getServiceProfileWithServiceType(serviceType).scpd();
	}
	bool UPnPDeviceProfile::match(const string & st) {
		if (st == "upnp:rootdevice") {
			return true;
		}

		for (vector<string>::iterator iter = _deviceTypes.begin(); iter != _deviceTypes.end(); iter++) {
			if (*iter == st) {
				return true;
			}
		}

		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin(); iter != _serviceProfiles.end(); iter++) {
			if (iter->serviceType() == st) {
				return true;
			}
		}
		
		return false;
	}
	bool UPnPDeviceProfile::hasServiceWithServiceType(const string & serviceType) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin(); iter != _serviceProfiles.end(); iter++) {
			if (iter->serviceType() == serviceType) {
				return true;
			}
		}
		return false;
	}
	bool UPnPDeviceProfile::hasServiceWithScpdUrl(const string & scpdUrl) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin(); iter != _serviceProfiles.end(); iter++) {
			if (iter->scpdUrl() == scpdUrl) {
				return true;
			}
		}
		return false;
	}
	bool UPnPDeviceProfile::hasServiceWithControlUrl(const string & controlUrl) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin(); iter != _serviceProfiles.end(); iter++) {
			if (iter->controlUrl() == controlUrl) {
				return true;
			}
		}
		return false;
	}
	bool UPnPDeviceProfile::hasServiceWithEventSubUrl(const string & eventSubUrl) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin(); iter != _serviceProfiles.end(); iter++) {
			if (iter->eventSubUrl() == eventSubUrl) {
				return true;
			}
		}
		return false;
	}
	UPnPServiceProfile & UPnPDeviceProfile::getServiceProfileWithServiceType(const string & serviceType) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin(); iter != _serviceProfiles.end(); iter++) {
			if (iter->serviceType() == serviceType) {
				return *iter;
			}
		}
		throw OS::Exception("not found service", -1, 0);
	}
	UPnPServiceProfile & UPnPDeviceProfile::getServiceProfileWithScpdUrl(const string & scpdUrl) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin(); iter != _serviceProfiles.end(); iter++) {
			if (iter->scpdUrl() == scpdUrl) {
				return *iter;
			}
		}
		throw OS::Exception("not found service", -1, 0);
	}
	UPnPServiceProfile & UPnPDeviceProfile::getServiceProfileWithControlUrl(const string & controlUrl) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin(); iter != _serviceProfiles.end(); iter++) {
			if (iter->controlUrl() == controlUrl) {
				return *iter;
			}
		}
		throw OS::Exception("not found service", -1, 0);
	}
	UPnPServiceProfile & UPnPDeviceProfile::getServiceProfileWithEventSubUrl(const string & eventSubUrl) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin(); iter != _serviceProfiles.end(); iter++) {
			if (iter->eventSubUrl() == eventSubUrl) {
				return *iter;
			}
		}
		throw OS::Exception("not found service", -1, 0);
	}
}
