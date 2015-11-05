#include "UPnPDevicePool.hpp"

namespace UPNP {
    
    using namespace std;

    UPnPDevicePool::UPnPDevicePool() : deviceTableLock(1) {
    }
    
    UPnPDevicePool::~UPnPDevicePool() {
    }
    
    void UPnPDevicePool::clear() {
        deviceTableLock.wait();
        deviceTable.clear();
        deviceTableLock.post();
    }
    
    UPnPDevice & UPnPDevicePool::getDevice(string udn) {
        return deviceTable[udn];
    }
    
    bool UPnPDevicePool::hasDevice(string udn) {
        return deviceTable.find(udn) != deviceTable.end();
    }

    void UPnPDevicePool::addDevice(UPnPDevice & device) {
        deviceTableLock.wait();
        deviceTable[device.getUdn()] = device;
        deviceTableLock.post();
    }

    void UPnPDevicePool::updateDevice(UPnPDevice & device) {
        deviceTableLock.wait();
        deviceTable[device.getUdn()] = device;
        deviceTableLock.post();
    }

    void UPnPDevicePool::removeDevice(string udn) {
        deviceTableLock.wait();
        deviceTable.erase(udn);
        deviceTableLock.post();
    }
    
    UPnPService * UPnPDevicePool::traverseService(const UPnPDevice & device, const UPnPServicePosition & servicePosition) {
        servicePosition.resetTraverse();
        const UPnPDevice * currentDevice = &device;
        while (servicePosition.hasNextDevice()) {
            if (!currentDevice) {
                throw -1;
                //                return NULL;
            }
            size_t index = servicePosition.traverseDevice();
            if (index >= currentDevice->getEmbeddedDevices().size()) {
                throw -1;
                //                return NULL;
            }
            currentDevice = &(currentDevice->getEmbeddedDevice(index));
        }
        if (servicePosition.getServiceIndex() >= currentDevice->getServices().size()) {
            throw -1;
            //            return NULL;
        }
        return &(currentDevice->getService(servicePosition.getServiceIndex()));
    }
    
    void UPnPDevicePool::bindScpd(const UPnPServicePosition & servicePosition, const Scpd & scpd) {
        deviceTableLock.wait();
        string udn = servicePosition.getUdn();
        if (hasDevice(udn)) {
            UPnPService * service = traverseService(getDevice(udn), servicePosition);
            if (service) {
                service->setScpd(scpd);
            }
        }
        deviceTableLock.post();
    }

	void UPnPDevicePool::cacheUpdate(const string & udn, unsigned long timeoutMilli) {
		deviceTableLock.wait();
		UPnPDevice & device = getDevice(udn);
		device.renew();
		device.setTimeout(timeoutMilli);
		deviceTableLock.post();
	}
	
}
