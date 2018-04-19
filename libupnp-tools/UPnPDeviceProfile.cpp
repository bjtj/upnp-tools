#include "UPnPDeviceProfile.hpp"
#include "UPnPDeviceSerializer.hpp"

namespace UPNP {

	using namespace std;
	using namespace OS;


	UPnPDeviceProfile::UPnPDeviceProfile()
		: _enabled(false) {
	}

	UPnPDeviceProfile::UPnPDeviceProfile(AutoRef<UPnPDevice> device)
		: _enabled(false), _device(device)
	{
	}
	
	UPnPDeviceProfile::~UPnPDeviceProfile() {
	}

	bool & UPnPDeviceProfile::enabled() {
		return _enabled;
	}

	bool UPnPDeviceProfile::enabled() const {
		return _enabled;
	}
	
	UDN UPnPDeviceProfile::udn() const {
		return _device->udn();
	}

	void UPnPDeviceProfile::setUdn(const UDN & udn) {
		_device->setUdn(udn);
	}

	string UPnPDeviceProfile::deviceDescription() const {
		return UPnPDeviceSerializer::serializeDeviceDescription(*(&_device));
	}

	AutoRef<UPnPDevice> & UPnPDeviceProfile::device() {
		return _device;
	}

	vector<string> UPnPDeviceProfile::deviceTypes() const {
		vector<string> types;
		types.push_back(_device->deviceType());
		vector< AutoRef<UPnPDevice> > devices = _device->allChildDevices();
		for (vector< AutoRef<UPnPDevice> >::iterator iter = devices.begin();
			 iter != devices.end(); iter++)
		{
			types.push_back((*iter)->deviceType());
		}
		return types;
	}
	
	vector< AutoRef<UPnPService> > UPnPDeviceProfile::allServices() const {
		return _device->allServices();
	}

	vector<string> UPnPDeviceProfile::serviceTypes() const {
		vector<string> types;
		vector< AutoRef<UPnPService> > services = allServices();
		for (vector< AutoRef<UPnPService> >::iterator iter = services.begin();
			 iter != services.end(); iter++)
		{
			AutoRef<UPnPService> service = *iter;
			types.push_back(service->serviceType());
		}
		return types;
	}
	
	AutoRef<UPnPService> UPnPDeviceProfile::getService(const string & serviceType) {
		vector< AutoRef<UPnPService> > services = allServices();
		for (vector< AutoRef<UPnPService> >::iterator iter = services.begin();
			 iter != services.end(); iter++)
		{
			AutoRef<UPnPService> & service = *iter;
			if (service->serviceType() == serviceType) {
				return service;
			}
		}
		return AutoRef<UPnPService>();
	}
	
	AutoRef<UPnPDevice> UPnPDeviceProfile::getDeviceByType(const string & deviceType) {
		if (_device->deviceType() == deviceType) {
			return _device;
		}
		vector< AutoRef<UPnPDevice> > devices = _device->allChildDevices();
		for (vector< AutoRef<UPnPDevice> >::iterator iter = devices.begin();
			 iter != devices.end(); ++iter)
		{
			AutoRef<UPnPDevice> device = *iter;
			if (device->deviceType() == deviceType) {
				return device;
			}
		}
		return AutoRef<UPnPDevice>();
	}
}
