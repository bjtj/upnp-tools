#ifndef __UPNP_SESSION_HPP__
#define __UPNP_SESSION_HPP__

#include <string>
#include <vector>
#include <map>
#include <liboslayer/os.hpp>
#include <liboslayer/StringElement.hpp>
#include <liboslayer/XmlParser.hpp>
#include "SSDPHeader.hpp"
#include "UPnPModels.hpp"
#include "HttpUtils.hpp"
#include "XmlUtils.hpp"

namespace UPNP {

	/**
	 *
	 */
	class UPnPSession {
	private:
		std::string udn;
		bool _completed;
		UTIL::AutoRef<UPnPDevice> rootDevice;
		unsigned long creationTime;
		unsigned long updateTime;
		unsigned long sessionTimeout;
	
	public:
		UPnPSession(const std::string & udn);
		virtual ~UPnPSession();
		void setCreationTime(unsigned long creationTime);
		void setUpdateTime(unsigned long updateTime);
		void setSessionTimeout(unsigned long sessionTimeout);
		unsigned long lifetime();
		unsigned long duration();
		bool outdated();
		std::string getDump(const HTTP::Url & url);
		void buildDevice(SSDP::SSDPHeader & header);
		std::string getFriendlyName();
		bool completed();
		UTIL::AutoRef<UPnPDevice> getRootDevice();
		std::string toString();
		std::string toString(UPnPDevice & device, int depth);
	};
}

#endif
