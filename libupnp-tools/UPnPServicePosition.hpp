#ifndef __UPNP_SERVICE_POSITION_HPP__
#define __UPNP_SERVICE_POSITION_HPP__

#include <vector>
#include <string>
#include "UPnPService.hpp"

namespace UPNP {
    
    class UPnPServicePosition {
    private:
        mutable std::string udn;
        mutable std::vector<size_t> deviceIndices;
        mutable size_t serviceIndex;
        mutable std::string serviceType;
        mutable size_t traverseDepth;
        mutable std::string scpdurl;
        
    public:
        UPnPServicePosition();
        virtual ~UPnPServicePosition();
        
        void setUdn(const std::string & udn) const;
        std::string & getUdn() const;
        void enterDevice(size_t index) const;
        void resetTraverse() const;
        size_t traverseDevice() const;
        bool hasNextDevice() const;
        size_t getServiceIndex() const;
        void setDeviceIndices(std::vector<size_t> & indices);
        void setSerivceIndex(size_t index);
        void setScpdUrl(const std::string & scpdurl);
        std::string getScpdUrl();
        std::string toString() const;
        void setServiceType(std::string serviceType);
    };
    
    
    class UPnPServicePositionMaker {
    private:
        std::string udn;
        std::vector<size_t> deviceIndices;
        size_t currentLevel;
    public:
        UPnPServicePositionMaker(std::string udn);
        virtual ~UPnPServicePositionMaker();
        UPnPServicePosition makeUPnPServicePosition(size_t index, UPnPService & service);
        void enter();
        void setDeviceIndex(size_t index);
        void leave();
    };
}

#endif
