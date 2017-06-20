#ifndef __SHARED_UPNP_DEVICE_LIST_HPP__
#define __SHARED_UPNP_DEVICE_LIST_HPP__

#include <liboslayer/os.hpp>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/Semaphore.hpp>
#include "UPnPModels.hpp"
#include <string>
#include <vector>

namespace UPNP {

	/**
	 * 
	 */
	class SharedUPnPDeviceList {
	private:
		std::vector<OS::AutoRef<UPnPDevice> > _devices;
		OS::Semaphore sem;
	public:
		SharedUPnPDeviceList();
		virtual ~SharedUPnPDeviceList();

		virtual void lock();
		virtual void unlock();

		std::vector<OS::AutoRef<UPnPDevice> > list();
		std::vector<OS::AutoRef<UPnPDevice> > list_s();
		OS::AutoRef<UPnPDevice> findByUdn(const UDN & udn);
		OS::AutoRef<UPnPDevice> findByUdn_s(const UDN & udn);
		void add(OS::AutoRef<UPnPDevice> device);
		void add_s(OS::AutoRef<UPnPDevice> device);
		void remove(OS::AutoRef<UPnPDevice> device);
		void remove_s(OS::AutoRef<UPnPDevice> device);
		size_t size();
		size_t size_s();
	};
}

#endif
