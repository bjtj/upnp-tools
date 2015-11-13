#ifndef __UPNP_DEVICE_ITERATOR_HPP__
#define __UPNP_DEVICE_ITERATOR_HPP__

#include "UPnPDevice.hpp"
#include <liboslayer/Iteration.hpp>

namespace UPNP {


	class UPnPDeviceIterator {
	private:

	public:
		UPnPDeviceIterator(UPnPDevice & deivce);
		virtual ~UPnPDeviceIterator();

		static void iterateDevicesRecursive(UPnPDevice & device, UTIL::IteratorHandler<UPnPDevice> & handler);
		static void iterateServices(UPnPDevice & device, UTIL::IteratorHandler<UPnPService> & handler);
		static void iterateServicesRecursive(UPnPDevice & device, UTIL::IteratorHandler<UPnPService> & handler);
	};
}

#endif