#include "UPnPDevicePool.hpp"
#include "macros.hpp"

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

    void UPnPDevicePool::addDevice(const UPnPDevice & device) {
        deviceTableLock.wait();
        deviceTable[device.getUdn()] = device;
        deviceTableLock.post();
    }

    void UPnPDevicePool::updateDevice(const UPnPDevice & device) {
        deviceTableLock.wait();
        deviceTable[device.getUdn()] = device;
        deviceTableLock.post();
    }

    void UPnPDevicePool::removeDevice(const string & udn) {
        deviceTableLock.wait();
        deviceTable.erase(udn);
        deviceTableLock.post();
    }
    
    UPnPService * UPnPDevicePool::traverseService(UPnPDevice & device, const UPnPServicePosition & servicePosition) {
        servicePosition.resetTraverse();
        UPnPDevice * currentDevice = &device;
        while (servicePosition.hasNextDevice()) {
            if (!currentDevice) {
                return NULL;
            }
            size_t index = servicePosition.traverseDevice();
            if (index >= currentDevice->getEmbeddedDevices().size()) {
                return NULL;
            }
            currentDevice = &(currentDevice->getEmbeddedDevice(index));
        }
        if (servicePosition.getServiceIndex() >= currentDevice->getServices().size()) {
            return NULL;
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

    vector<UPnPDevice> UPnPDevicePool::getRootDevices() const {
        vector<UPnPDevice> ret;
        deviceTableLock.wait();
        LOOP_CONST_MAP(deviceTable, string, UPnPDevice, iter) {
            if (iter->second.isRootDevice()) {
                ret.push_back(iter->second);
            }
        }
        deviceTableLock.post();
        return ret;
    }
    vector<UPnPDevice> UPnPDevicePool::getWholeDevices() const {
        vector<UPnPDevice> ret;
        deviceTableLock.wait();
        LOOP_CONST_MAP(deviceTable, string, UPnPDevice, iter) {
            if (iter->second.isRootDevice()) {
                const UPnPDevice & device = iter->second;
                ret.push_back(iter->second);
                vector<UPnPDevice> embeddedDevices = getDevicesRecursive(device);
                ret.insert(ret.end(), embeddedDevices.begin(), embeddedDevices.end());
            }
        }
        deviceTableLock.post();
        return ret;
    }
    std::vector<UPnPDevice> UPnPDevicePool::getDevicesRecursive(const UPnPDevice & device) const {
        vector<UPnPDevice> ret;
        deviceTableLock.wait();
        LOOP_CONST_MAP(deviceTable, string, UPnPDevice, iter) {
            const UPnPDevice & item = iter->second;
            ret.push_back(item);
            const vector<UPnPDevice> & embeddedDevices = iter->second.getEmbeddedDevices();
            ret.insert(ret.end(), embeddedDevices.begin(), embeddedDevices.end());
            LOOP_VEC(embeddedDevices, i) {
                vector<UPnPDevice> itemRet = getDevicesRecursive(embeddedDevices[i]);
                ret.insert(ret.end(), itemRet.begin(), itemRet.end());
            }
        }
        deviceTableLock.post();
        return ret;
    }
    std::vector<UPnPService> UPnPDevicePool::getWholeServices() const {
        vector<UPnPService> ret;
        deviceTableLock.wait();
        // TODO: get whole services
        deviceTableLock.post();
        return ret;
    }
}
