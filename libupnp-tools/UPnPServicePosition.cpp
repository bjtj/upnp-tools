#include "UPnPServicePosition.hpp"
#include <liboslayer/os.hpp>
#include <liboslayer/Text.hpp>
#include "macros.hpp"

namespace UPNP {
    
    using namespace std;
    using namespace OS;
    using namespace UTIL;
    
    ServicePosition::ServicePosition() : traverseDepth(0), serviceIndex(0) {
    }
    
    ServicePosition::~ServicePosition() {
    }
    
    void ServicePosition::setUdn(const string & udn) const {
        this->udn = udn;
    }
    string & ServicePosition::getUdn() const {
        return udn;
    }
    
    void ServicePosition::enterDevice(size_t index) const {
        deviceIndices.push_back(index);
    }
    
    void ServicePosition::resetTraverse() const {
        traverseDepth = 0;
    }
    
    size_t ServicePosition::traverseDevice() const {
        if (!hasNextDevice()) {
            throw Exception("no more index", -1, 0);
        }
        return deviceIndices[traverseDepth++];
    }
    
    bool ServicePosition::hasNextDevice() const {
        return traverseDepth < deviceIndices.size();
    }
    
    size_t ServicePosition::getServiceIndex() const {
        return serviceIndex;
    }
    
    void ServicePosition::setDeviceIndices(vector<size_t> & indices) {
        this->deviceIndices = indices;
    }
    
    void ServicePosition::setSerivceIndex(size_t index) {
        this->serviceIndex = index;
    }
    
    void ServicePosition::setScpdUrl(const string & scpdurl) {
        this->scpdurl = scpdurl;
    }
    
    string ServicePosition::getScpdUrl() {
        return scpdurl;
    }
    
    string ServicePosition::toString() const {
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
    
    void ServicePosition::setServiceType(string serviceType) {
        this->serviceType = serviceType;
    }
    
 
    ServicePositionMaker::ServicePositionMaker(string udn) : udn(udn), currentLevel(0) {
    }
    
    ServicePositionMaker::~ServicePositionMaker() {
    }
    
    ServicePosition ServicePositionMaker::makeServicePosition(size_t index, UPnPService & service) {
        ServicePosition sp;
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
    
    void ServicePositionMaker::enter() {
        currentLevel++;
        if (currentLevel >= deviceIndices.size()) {
            deviceIndices.push_back(0);
        }
    }
    
    void ServicePositionMaker::setDeviceIndex(size_t index) {
        deviceIndices[currentLevel - 1] = index;
    }
    
    void ServicePositionMaker::leave() {
        currentLevel--;
    }
}