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
	
	string UPnPDeviceProfile::const_uuid() const {
		return _uuid;
	}
	
	string & UPnPDeviceProfile::deviceDescription() {
		return _deviceDescription;
	}
	
	string UPnPDeviceProfile::const_deviceDescription() const {
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
		return getServiceProfileByServiceType(serviceType).scpd();
	}
	
	bool UPnPDeviceProfile::hasDeviceType(const string & deviceType) {
		for (vector<string>::iterator iter = _deviceTypes.begin();
			 iter != _deviceTypes.end(); iter++) {
			if (*iter == deviceType) {
				return true;
			}
		}
		return false;
	}

	bool UPnPDeviceProfile::hasServiceByServiceType(const string & serviceType) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin();
			 iter != _serviceProfiles.end(); iter++) {
			if (iter->serviceType() == serviceType) {
				return true;
			}
		}
		return false;
	}
	
	bool UPnPDeviceProfile::hasServiceByScpdUrl(const string & scpdUrl) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin();
			 iter != _serviceProfiles.end(); iter++) {
			if (iter->scpdUrl() == scpdUrl) {
				return true;
			}
		}
		return false;
	}
	
	bool UPnPDeviceProfile::hasServiceByControlUrl(const string & controlUrl) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin();
			 iter != _serviceProfiles.end(); iter++) {
			
			if (iter->controlUrl() == controlUrl) {
				return true;
			}
		}
		return false;
	}
	
	bool UPnPDeviceProfile::hasServiceByEventSubUrl(const string & eventSubUrl) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin();
			 iter != _serviceProfiles.end(); iter++) {
			
			if (iter->eventSubUrl() == eventSubUrl) {
				return true;
			}
		}
		return false;
	}
	UPnPServiceProfile & UPnPDeviceProfile::getServiceProfileByServiceType(const string & serviceType) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin();
			 iter != _serviceProfiles.end(); iter++) {
			if (iter->serviceType() == serviceType) {
				return *iter;
			}
		}
		throw OS::Exception("not found service", -1, 0);
	}
	
	UPnPServiceProfile & UPnPDeviceProfile::getServiceProfileByScpdUrl(const string & scpdUrl) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin();
			 iter != _serviceProfiles.end(); iter++) {
			if (iter->scpdUrl() == scpdUrl) {
				return *iter;
			}
		}
		throw OS::Exception("not found service", -1, 0);
	}
	
	UPnPServiceProfile & UPnPDeviceProfile::getServiceProfileByControlUrl(const string & controlUrl) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin();
			 iter != _serviceProfiles.end(); iter++) {
			if (iter->controlUrl() == controlUrl) {
				return *iter;
			}
		}
		throw OS::Exception("not found service", -1, 0);
	}
	UPnPServiceProfile & UPnPDeviceProfile::getServiceProfileByEventSubUrl(const string & eventSubUrl) {
		for (vector<UPnPServiceProfile>::iterator iter = _serviceProfiles.begin();
			 iter != _serviceProfiles.end(); iter++) {
			if (iter->eventSubUrl() == eventSubUrl) {
				return *iter;
			}
		}
		throw OS::Exception("not found service", -1, 0);
	}
}
