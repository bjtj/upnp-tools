#ifndef __UPNP_LOCATION_RESOLVER_HPP__
#define __UPNP_LOCATION_RESOLVER_HPP__

#include "UPnPModels.hpp"

namespace UPNP {
	
	class UPnPLocationResolver {
	public:
		UPnPLocationResolver();
		virtual ~UPnPLocationResolver();
		virtual std::string makeScpdUrl(OS::AutoRef<UPnPDevice> device, OS::AutoRef<UPnPService> service);
		virtual std::string makeControlUrl(OS::AutoRef<UPnPDevice> device, OS::AutoRef<UPnPService> service);
		virtual std::string makeEventSubUrl(OS::AutoRef<UPnPDevice> device, OS::AutoRef<UPnPService> service);
		virtual std::string generalize(const std::string & url);
		void resolve(OS::AutoRef<UPnPDevice> device);
		void resolveRecursive(OS::AutoRef<UPnPDevice> device);
	};
}

#endif
