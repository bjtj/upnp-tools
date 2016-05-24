#include "UPnPDeviceProfileBuilder.hpp"
#include "UPnPDeviceDeserializer.hpp"
#include "UPnPDeviceSerializer.hpp"
#include <liboslayer/XmlParser.hpp>

namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace UTIL;

	UPnPDeviceProfileBuilder::UPnPDeviceProfileBuilder(const string uuid, AutoRef<UPnPDevice> device) : _uuid(uuid), _device(device) {
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
		deviceProfile.uuid() = _uuid;
		setUdn(_device, "uuid:" + _uuid);
		deviceProfile.deviceDescription() = UPnPDeviceSerializer::serializeDeviceDescription(*_device);

		vector<AutoRef<UPnPDevice> > devices;
		devices.push_back(_device);
		vector<AutoRef<UPnPDevice> > embeds = selectAllDevices(_device);
		devices.insert(devices.end(), embeds.begin(), embeds.end());
		
		for (vector<AutoRef<UPnPDevice> >::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			deviceProfile.deviceTypes().push_back((*iter)->getDeviceType());

			vector<AutoRef<UPnPService> > & services = (*iter)->services();
			for (vector<AutoRef<UPnPService> >::iterator si = services.begin(); si != services.end(); si++) {

				AutoRef<UPnPService> service = *si;
				UPnPServiceProfile serviceProfile;
				
				serviceProfile.scpd() = UPnPDeviceSerializer::serializeScpd(*service);
				serviceProfile.serviceType() = service->getServiceType();
				serviceProfile.serviceId() = service->getServiceId();
				serviceProfile.scpdUrl() = service->getScpdUrl();
				serviceProfile.controlUrl() = service->getControlUrl();
				serviceProfile.eventSubUrl() = service->getEventSubUrl();
				deviceProfile.serviceProfiles().push_back(serviceProfile);
			}
		}

		return deviceProfile;
	}

	void UPnPDeviceProfileBuilder::setUdn(AutoRef<UPnPDevice> device, const string & udn) {
		device->setUdn(udn);
		for (size_t i = 0; i < device->embeddedDevices().size(); i++) {
			setUdn(device->embeddedDevices()[i], udn);
		}
	}
}
