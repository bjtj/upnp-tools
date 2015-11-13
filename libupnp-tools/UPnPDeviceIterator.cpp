#include "UPnPDeviceIterator.hpp"
#include <vector>

namespace UPNP {

	using namespace std;
	using namespace UTIL;

	UPnPDeviceIterator::UPnPDeviceIterator(UPnPDevice & deivce) {
	}
	UPnPDeviceIterator::~UPnPDeviceIterator() {
	}

	void UPnPDeviceIterator::iterateDevicesRecursive(UPnPDevice & device, IteratorHandler<UPnPDevice> & handler) {

		handler.onItem(device);

		vector<UPnPDevice> embeds = device.getEmbeddedDevices();
		for (size_t i = 0; i < embeds.size(); i++) {
			UPnPDevice & embed = embeds[i];
			iterateDevicesRecursive(embed, handler);
		}
	}
	void UPnPDeviceIterator::iterateServices(UPnPDevice & device, IteratorHandler<UPnPService> & handler) {

		vector<UPnPService> & services = device.getServices();
		for (size_t i = 0; i < services.size(); i++) {
			handler.onItem(services[i]);
		}
	}
	void UPnPDeviceIterator::iterateServicesRecursive(UPnPDevice & device, IteratorHandler<UPnPService> & handler) {

		iterateServices(device, handler);

		vector<UPnPDevice> embeds = device.getEmbeddedDevices();
		for (size_t i = 0; i < embeds.size(); i++) {
			UPnPDevice & embed = embeds[i];
			iterateServicesRecursive(embed, handler);
		}

	}
}