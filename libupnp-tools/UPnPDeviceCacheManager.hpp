#ifndef __UPNP_DEVICE_CACHE_MANAGER_HPP__
#define __UPNP_DEVICE_CACHE_MANAGER_HPP__

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
		std::map<std::string, UPnPDeviceCache> _cache;
	public:
		UPnPDeviceCacheManager();
		virtual ~UPnPDeviceCacheManager();
		std::vector<std::string> udn_set();
		void put(const OS::AutoRef<UPnPDevice> & device);
		OS::AutoRef<UPnPDevice> & get(const std::string & udn);
		void collect();
	};
}

#endif
