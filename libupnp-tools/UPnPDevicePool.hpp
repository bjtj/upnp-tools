#ifndef __UPNP_DEVICE_POLL_HPP__
#define __UPNP_DEVICE_POLL_HPP__

#include <vector>
#include <string>
#include <liboslayer/os.hpp>
#include "UPnPDevice.hpp"
#include "UPnPServicePosition.hpp"

namespace UPNP {

    class UPnPDevicePool {
    private:
        OS::Semaphore deviceTableLock;
        std::map<std::string, UPnPDevice> deviceTable;
        
    public:
        UPnPDevicePool();
        virtual ~UPnPDevicePool();
        void clear();
        UPnPDevice & getDevice(std::string udn);
        bool hasDevice(std::string udn);
        void addDevice(const UPnPDevice & device);
        void updateDevice(const UPnPDevice & device);
        void removeDevice(const std::string & udn);
        UPnPService * traverseService(UPnPDevice & device, const UPnPServicePosition & servicePosition);
        void bindScpd(const UPnPServicePosition & servicePosition, const Scpd & scpd);
		void cacheUpdate(const std::string & udn, unsigned long timeoutMilli);
        
        std::vector<UPnPDevice> getRootDevices() const;
        std::vector<UPnPDevice> getWholeDevices() const;
        std::vector<UPnPDevice> getDevicesRecursive(const UPnPDevice & device) const;
        std::vector<UPnPService> getWholeServices() const;
        
        void lock();
        void unlock();
    };

}

#endif
