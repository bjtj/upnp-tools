#include "UPnPActionInvoke.hpp"

namespace UPNP {
    
    using namespace std;
    using namespace UTIL;
    
    /**
     * @brief invoek action response listener
     */
    
    UPnPActionParameters::UPnPActionParameters() {
    }
    UPnPActionParameters::~UPnPActionParameters() {
    }
    size_t UPnPActionParameters::size() const {
        return params.size();
    }
    const string & UPnPActionParameters::operator[] (const string & name) const {
        return params[name];
    }
    string & UPnPActionParameters::operator[] (const string & name) {
        return params[name];
    }
    const NameValue & UPnPActionParameters::operator[] (size_t index) const {
        return params[index];
    }
    NameValue & UPnPActionParameters::operator[] (size_t index) {
        return params[index];
    }
    
    /**
     *
     */
    
    UPnPActionRequest::UPnPActionRequest() {
        
    }
    
    UPnPActionRequest::~UPnPActionRequest() {
        
    }
    
    /**
     *
     */
    
    UPnPActionResponse::UPnPActionResponse() {
        
    }
    
    UPnPActionResponse::~UPnPActionResponse() {
        
    }
}