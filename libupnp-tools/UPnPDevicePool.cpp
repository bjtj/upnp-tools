#include "UPnPDevicePool.hpp"

namespace UPNP {
    
    using namespace std;

    DevicePool::DevicePool() : deviceTableLock(1) {
    }
    
    DevicePool::~DevicePool() {
    }
    
    void DevicePool::clear() {
        deviceTableLock.wait();
        deviceTable.clear();
        deviceTableLock.post();
    }
    
    UPnPDevice & DevicePool::getDevice(string udn) {
        return deviceTable[udn];
    }
    
    bool DevicePool::hasDevice(string udn) {
        return deviceTable.find(udn) != deviceTable.end();
    }
    void DevicePool::addDevice(UPnPDevice & device) {
        deviceTableLock.wait();
        deviceTable[device.getUdn()] = device;
        deviceTableLock.post();
    }
    void DevicePool::updateDevice(UPnPDevice & device) {
        deviceTableLock.wait();
        deviceTable[device.getUdn()] = device;
        deviceTableLock.post();
    }
    
    void DevicePool::removeDevice(string udn) {
        deviceTableLock.wait();
        deviceTable.erase(udn);
        deviceTableLock.post();
    }
    
    UPnPService * DevicePool::traverseService(const UPnPDevice & device, const ServicePosition & servicePosition) {
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
    
    void DevicePool::bindScpd(const ServicePosition & servicePosition, const Scpd & scpd) {
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
	
}
