#ifndef __UPNP_DEVICE_PROFILE_SESSION_HPP__
#define __UPNP_DEVICE_PROFILE_SESSION_HPP__

#include "UPnPDeviceProfile.hpp"

namespace UPNP {
	
	class UPnPDeviceProfileSession {
	private:
		bool enabled;
		UPnPDeviceProfile _profile;
	public:
		UPnPDeviceProfileSession(const UPnPDeviceProfile & profile);
		virtual ~UPnPDeviceProfileSession();
		UPnPDeviceProfile & profile();
		void setEnable(bool enable);
		bool isEnabled();
	};
}

#endif
