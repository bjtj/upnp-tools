#ifndef __UPNP_RESOURCE_MANAGER_HPP__
#define __UPNP_RESOURCE_MANAGER_HPP__

#include <string>
#include <libhttp-server/Url.hpp>

namespace UPNP {
	
	class UPnPResourceManager {
	private:
	public:
		UPnPResourceManager();
		virtual ~UPnPResourceManager();
		static std::string getResource(const HTTP::Url & url);
	};
}

#endif
