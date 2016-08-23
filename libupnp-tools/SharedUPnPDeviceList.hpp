#ifndef __SHARED_UPNP_DEVICE_LIST_HPP__
#define __SHARED_UPNP_DEVICE_LIST_HPP__

#include <liboslayer/os.hpp>
#include <liboslayer/AutoRef.hpp>
#include "UPnPModels.hpp"
#include <string>
#include <vector>

namespace UPNP {

	/**
	 * 
	 */
	class SharedUPnPDeviceList {
	private:
		std::vector<UTIL::AutoRef<UPnPDevice> > _devices;
		OS::Semaphore sem;
	public:
		SharedUPnPDeviceList();
		virtual ~SharedUPnPDeviceList();

		virtual void lock();
		virtual void unlock();

		std::vector<UTIL::AutoRef<UPnPDevice> > list();
		std::vector<UTIL::AutoRef<UPnPDevice> > list_s();
		UTIL::AutoRef<UPnPDevice> findByUdn(const std::string & udn);
		UTIL::AutoRef<UPnPDevice> findByUdn_s(const std::string & udn);
		void add(UTIL::AutoRef<UPnPDevice> device);
		void add_s(UTIL::AutoRef<UPnPDevice> device);
		void remove(UTIL::AutoRef<UPnPDevice> device);
		void remove_s(UTIL::AutoRef<UPnPDevice> device);
		size_t size();
		size_t size_s();
	};
}

#endif
