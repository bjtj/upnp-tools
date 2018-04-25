#ifndef __SHARED_UPNP_DEVICE_LIST_HPP__
#define __SHARED_UPNP_DEVICE_LIST_HPP__

#include <liboslayer/os.hpp>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/Semaphore.hpp>
#include "UPnPModels.hpp"
#include <string>
#include <vector>

namespace upnp {

	/**
	 * 
	 */
	class SharedUPnPDeviceList {
	private:
		std::vector<osl::AutoRef<UPnPDevice> > _devices;
		osl::Semaphore sem;
	public:
		SharedUPnPDeviceList();
		virtual ~SharedUPnPDeviceList();
		virtual void lock();
		virtual void unlock();
		std::vector< osl::AutoRef<UPnPDevice> > list();
		void list(std::vector< osl::AutoRef<UPnPDevice> > & vec);
		osl::AutoRef<UPnPDevice> findByUdn(const UDN & udn);
		void add(osl::AutoRef<UPnPDevice> device);
		void remove(osl::AutoRef<UPnPDevice> device);
		size_t size();
	};
}

#endif
