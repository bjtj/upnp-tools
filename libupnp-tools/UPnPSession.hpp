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
		static const unsigned long DEFAULT_TIMEOUT = 1800 * 1000; 
		std::string _udn;
		bool _completed;
		UTIL::AutoRef<UPnPDevice> rootDevice;
		unsigned long creationTime;
		unsigned long updateTime;
		unsigned long sessionTimeout;
	
	public:
		UPnPSession(const std::string & udn);
		virtual ~UPnPSession();
		std::string & udn();
		void setCreationTime(unsigned long creationTime);
		void setUpdateTime(unsigned long updateTime);
		void setSessionTimeout(unsigned long sessionTimeout);
		unsigned long lifetimeFromCreation();
		unsigned long lifetimeFromLastUpdate();
		bool outdated();
		void prolong(const std::string & cacheControl);
		void prolong(unsigned long timeout);
		std::string getDump(const HTTP::Url & url);
		void buildDevice(SSDP::SSDPHeader & header);
		std::string getFriendlyName();
		bool completed();
		UTIL::AutoRef<UPnPDevice> getRootDevice();
		unsigned long parseCacheControlMilli(const std::string & cacheControl, unsigned long def);
	};
}

#endif
