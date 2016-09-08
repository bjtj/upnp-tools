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

	UPnPDeviceProfileBuilder::UPnPDeviceProfileBuilder(const string uuid, AutoRef<UPnPDevice> device)
		: _uuid(uuid), _device(device), _locationResolver(new UPnPLocationResolver) {
	}
	UPnPDeviceProfileBuilder::~UPnPDeviceProfileBuilder() {
	}

	string & UPnPDeviceProfileBuilder::uuid() {
		return _uuid;
	}

	AutoRef<UPnPDevice> UPnPDeviceProfileBuilder::device() {
		return _device;
	}

	vector<AutoRef<UPnPDevice> > UPnPDeviceProfileBuilder::selectAllDevices(AutoRef<UPnPDevice> device) {
		vector<AutoRef<UPnPDevice> > ret;
		vector<AutoRef<UPnPDevice> > & children = device->embeddedDevices();
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
		if (_uuid.empty()) {
			Uuid uuid(_device->getUdn());
			_uuid = uuid.getUuid();
		} else {
			_device->setUdnRecursive("uuid:" + _uuid);
		}

		_locationResolver->resolveRecursive(_device);

		deviceProfile.uuid() = _uuid;
		deviceProfile.deviceDescription() = UPnPDeviceSerializer::serializeDeviceDescription(*_device);

		vector<AutoRef<UPnPDevice> > devices;
		devices.push_back(_device);
		vector<AutoRef<UPnPDevice> > embeds = selectAllDevices(_device);
		devices.insert(devices.end(), embeds.begin(), embeds.end());
		
		for (vector<AutoRef<UPnPDevice> >::iterator iter = devices.begin(); iter != devices.end(); iter++) {

			AutoRef<UPnPDevice> device = (*iter);
			deviceProfile.deviceTypes().push_back(device->getDeviceType());
			vector<AutoRef<UPnPService> > & services = device->services();
			
			for (vector<AutoRef<UPnPService> >::iterator si = services.begin(); si != services.end(); si++) {

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

	void UPnPDeviceProfileBuilder::setLocationResolver(UTIL::AutoRef<UPnPLocationResolver> locationResolver) {
		this->_locationResolver = locationResolver;
	}
}
