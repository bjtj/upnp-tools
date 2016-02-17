#ifndef __UPNP_SESSION_HPP__
#define __UPNP_SESSION_HPP__

#include <string>
#include "UPnPDevice.hpp"
#include "UPnPService.hpp"

namespace UPNP {

	class UPnPSession {
	private:
		
		std::string udn;
		UPnPDevice rootDevice;
		unsigned long timeout;
		unsigned long creationTime;
		
	public:
		
		UPnPSession();
		virtual ~UPnPSession();
		std::string getUdn();
		bool creationDone();
		void setUdn(const std::string & udn);
		void setDevice(UPnPDevice & device);
		void setService(std::string deviceType, UPnPService & service);
		UPnPDevice & getRootDevice();
		UPnPDevice & getDevice(const std::string & deviceType);
		UPnPService & getService(const std::string & deviceType, const std::string & serviceType);
		bool outdated();
	};

}

#endif
