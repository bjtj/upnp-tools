#include "UPnPDeviceProfileSession.hpp"

namespace UPNP {

	UPnPDeviceProfileSession::UPnPDeviceProfileSession(const UPnPDeviceProfile & profile) : enabled(false), _profile(profile){
	}
	UPnPDeviceProfileSession::~UPnPDeviceProfileSession() {
	}
	UPnPDeviceProfile & UPnPDeviceProfileSession::profile() {
		return _profile;
	}
	void UPnPDeviceProfileSession::setEnable(bool enable) {
		this->enabled = enable;
	}
	bool UPnPDeviceProfileSession::isEnabled() {
		return enabled;
	}
}
