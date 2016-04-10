#ifndef __UPNP_RESOURCE_MANAGER_HPP__
#define __UPNP_RESOURCE_MANAGER_HPP__

#include <string>
#include <liboslayer/Properties.hpp>
#include <libhttp-server/Url.hpp>

namespace UPNP {
	
	class UPnPResourceManager {
	private:
		static UTIL::Properties props;
	public:
		UPnPResourceManager();
		virtual ~UPnPResourceManager();
		static std::string getResource(const HTTP::Url & url);
		static UTIL::Properties & properties();
	};
}

#endif
