#include "UPnPServicePosition.hpp"
#include <liboslayer/os.hpp>
#include <liboslayer/Text.hpp>
#include "macros.hpp"

namespace UPNP {
    
    using namespace std;
    using namespace OS;
    using namespace UTIL;
    
    UPnPServicePosition::UPnPServicePosition() : traverseDepth(0), serviceIndex(0) {
    }
    
    UPnPServicePosition::~UPnPServicePosition() {
    }
    
    void UPnPServicePosition::setUdn(const string & udn) const {
        this->udn = udn;
    }
    string & UPnPServicePosition::getUdn() const {
        return udn;
    }
    
    void UPnPServicePosition::enterDevice(size_t index) const {
        deviceIndices.push_back(index);
    }
    
    void UPnPServicePosition::resetTraverse() const {
        traverseDepth = 0;
    }
    
    size_t UPnPServicePosition::traverseDevice() const {
        if (!hasNextDevice()) {
            throw Exception("no more index", -1, 0);
        }
        return deviceIndices[traverseDepth++];
    }
    
    bool UPnPServicePosition::hasNextDevice() const {
        return traverseDepth < deviceIndices.size();
    }
    
    size_t UPnPServicePosition::getServiceIndex() const {
        return serviceIndex;
    }
    
    void UPnPServicePosition::setDeviceIndices(vector<size_t> & indices) {
        this->deviceIndices = indices;
    }
    
    void UPnPServicePosition::setSerivceIndex(size_t index) {
        this->serviceIndex = index;
    }
    
    void UPnPServicePosition::setScpdUrl(const string & scpdurl) {
        this->scpdurl = scpdurl;
    }
    
    string UPnPServicePosition::getScpdUrl() {
        return scpdurl;
    }
    
    string UPnPServicePosition::toString() const {
        string str;
        LOOP_VEC(deviceIndices, i) {
            size_t x = deviceIndices[i];
            str.append(Text::toString((int)x));
            str.append(" ");
        }
        str.append("[" + Text::toString((int)serviceIndex) + "]");
        str.append(" - " + serviceType);
        return str;
    }
    
    void UPnPServicePosition::setServiceType(string serviceType) {
        this->serviceType = serviceType;
    }
    
 
    UPnPServicePositionMaker::UPnPServicePositionMaker(string udn) : udn(udn), currentLevel(0) {
    }
    
    UPnPServicePositionMaker::~UPnPServicePositionMaker() {
    }
    
    UPnPServicePosition UPnPServicePositionMaker::makeUPnPServicePosition(size_t index, UPnPService & service) {
        UPnPServicePosition sp;
        sp.setUdn(udn);
        sp.setScpdUrl(service["SCPDURL"]);
        sp.setServiceType(service["serviceType"]);
        if (currentLevel > 0) {
            vector<size_t> indices(deviceIndices.begin(), deviceIndices.begin() + currentLevel);
            sp.setDeviceIndices(indices);
        }
        sp.setSerivceIndex(index);
        return sp;
    }
    
    void UPnPServicePositionMaker::enter() {
        currentLevel++;
        if (currentLevel >= deviceIndices.size()) {
            deviceIndices.push_back(0);
        }
    }
    
    void UPnPServicePositionMaker::setDeviceIndex(size_t index) {
        deviceIndices[currentLevel - 1] = index;
    }
    
    void UPnPServicePositionMaker::leave() {
        currentLevel--;
    }
}