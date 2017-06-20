#ifndef __UPNP_DEVICE_CACHE_MANAGER_HPP__
#define __UPNP_DEVICE_CACHE_MANAGER_HPP__

#include "UPnPTerms.hpp"

namespace UPNP {

	class UPnPDeviceCache : public UPnPCache {
	private:
		OS::AutoRef<UPnPDevice> _device;
	public:
		UPnPDeviceCache(const OS::AutoRef<UPnPDevice> & device);
		virtual ~UPnPDeviceCache();
		OS::AutoRef<UPnPDevice> & device();
	};


	/**
	 * upnp device cache manager
	 */
	class UPnPDeviceCacheManager {
	private:
		std::map<UDN, UPnPDeviceCache> _cache;
	public:
		UPnPDeviceCacheManager();
		virtual ~UPnPDeviceCacheManager();
		std::vector<UDN> udn_set();
		void put(const OS::AutoRef<UPnPDevice> & device);
		OS::AutoRef<UPnPDevice> & get(const UDN & udn);
		void collect();
	};
}

#endif
