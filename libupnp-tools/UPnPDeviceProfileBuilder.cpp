#include <liboslayer/XmlParser.hpp>
#include <liboslayer/Uuid.hpp>
#include "UPnPDeviceProfileBuilder.hpp"
#include "UPnPDeviceDeserializer.hpp"
#include "UPnPDeviceSerializer.hpp"


namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace UTIL;

	UPnPDeviceProfileBuilder::UPnPDeviceProfileBuilder(AutoRef<UPnPDevice> device)
		: _device(device), _locationResolver(new UPnPLocationResolver) {
	}

	UPnPDeviceProfileBuilder::UPnPDeviceProfileBuilder(const UDN & udn, AutoRef<UPnPDevice> device)
		: _udn(udn), _device(device), _locationResolver(new UPnPLocationResolver) {
	}
	UPnPDeviceProfileBuilder::~UPnPDeviceProfileBuilder() {
	}

	UDN & UPnPDeviceProfileBuilder::udn() {
		return _udn;
	}

	AutoRef<UPnPDevice> UPnPDeviceProfileBuilder::device() {
		return _device;
	}

	vector<AutoRef<UPnPDevice> > UPnPDeviceProfileBuilder::selectAllDevices(AutoRef<UPnPDevice> device) {
		vector<AutoRef<UPnPDevice> > ret;
		vector<AutoRef<UPnPDevice> > & children = device->childDevices();
		for (vector<AutoRef<UPnPDevice> >::iterator iter = children.begin(); iter != children.end(); iter++) {
			ret.push_back(*iter);
			vector<AutoRef<UPnPDevice> > lst = selectAllDevices(*iter);
			ret.insert(ret.end(), lst.begin(), lst.end());
		}
		return ret;
	}

	UPnPDeviceProfile UPnPDeviceProfileBuilder::build() {
		if (_device.nil()) {
			throw Exception("device is null");
		}

		UPnPDeviceProfile deviceProfile;
		if (_udn.empty()) {
			_udn = _device->getUdn();
		} else {
			_device->setUdnRecursive(_udn);
		}
		_locationResolver->resolveRecursive(_device);
		deviceProfile.udn() = _udn;
		deviceProfile.deviceDescription() = UPnPDeviceSerializer::serializeDeviceDescription(*_device);

		vector<AutoRef<UPnPDevice> > devices;
		devices.push_back(_device);
		vector<AutoRef<UPnPDevice> > lst = selectAllDevices(_device);
		devices.insert(devices.end(), lst.begin(), lst.end());
		for (vector< AutoRef<UPnPDevice> >::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			AutoRef<UPnPDevice> device = (*iter);
			deviceProfile.deviceTypes().push_back(device->getDeviceType());
			vector<AutoRef<UPnPService> > & services = device->services();
			
			for (vector< AutoRef<UPnPService> >::iterator si = services.begin(); si != services.end(); si++) {
				AutoRef<UPnPService> service = *si;
				UPnPServiceProfile serviceProfile;
				serviceProfile.scpd() = UPnPDeviceSerializer::serializeScpd(service->scpd());
				serviceProfile.serviceType() = service->serviceType();
				serviceProfile.serviceId() = service->serviceId();
				serviceProfile.scpdUrl() = service->scpdUrl();
				serviceProfile.controlUrl() = service->controlUrl();
				serviceProfile.eventSubUrl() = service->eventSubUrl();
				deviceProfile.serviceProfiles().push_back(serviceProfile);
			}
		}
		return deviceProfile;
	}

	void UPnPDeviceProfileBuilder::setLocationResolver(AutoRef<UPnPLocationResolver> locationResolver) {
		this->_locationResolver = locationResolver;
	}
}
