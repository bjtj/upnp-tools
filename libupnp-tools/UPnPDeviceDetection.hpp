#ifndef __UPNP_DEVICE_DETECTION_HPP__
#define __UPNP_DEVICE_DETECTION_HPP__

#include <string>
#include <libhttp-server/HttpHeader.hpp>
#include "UPnPServicePosition.hpp"

namespace UPNP {

	class UPnPDeviceDetection {
	private:
	public:
		UPnPDeviceDetection();
		virtual ~UPnPDeviceDetection();
    
		virtual void onDeviceCacheUpdate(const HTTP::HttpHeader & header) = 0;
		virtual void onDeviceHelloWithUrl(const std::string & url, const HTTP::HttpHeader & header) = 0;
		virtual void onDeviceDescriptionInXml(std::string baseUrl, std::string xmlDoc) = 0;
		virtual void onScpdInXml(const UPnPServicePosition & servicePosition, std::string xmlDoc) = 0;
		virtual void onDeviceByeBye(std::string udn) = 0;
	};
}

#endif