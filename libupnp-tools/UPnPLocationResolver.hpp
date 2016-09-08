#ifndef __UPNP_LOCATION_RESOLVER_HPP__
#define __UPNP_LOCATION_RESOLVER_HPP__

#include "UPnPModels.hpp"

namespace UPNP {
	
	class UPnPLocationResolver {
	public:
		UPnPLocationResolver();
		virtual ~UPnPLocationResolver();
		virtual std::string makeScpdUrl(UTIL::AutoRef<UPnPDevice> device, UTIL::AutoRef<UPnPService> service);
		virtual std::string makeControlUrl(UTIL::AutoRef<UPnPDevice> device, UTIL::AutoRef<UPnPService> service);
		virtual std::string makeEventSubUrl(UTIL::AutoRef<UPnPDevice> device, UTIL::AutoRef<UPnPService> service);
		virtual std::string generalize(const std::string & url);
		void resolve(UTIL::AutoRef<UPnPDevice> device);
		void resolveRecursive(UTIL::AutoRef<UPnPDevice> device);
	};
}

#endif
