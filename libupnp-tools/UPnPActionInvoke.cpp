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
	vector<string> UPnPActionParameters::getParameterNames() const {
		vector<string> names;
		for (size_t i = 0; i < params.size(); i++) {
			const NameValue & nv = params.const_getByIndex(i);
			string name = nv.getName();
			names.push_back(name);
		}
		return names;
	}
	string & UPnPActionParameters::getParameter(const string & name) {
		return params[name];
	}
	string UPnPActionParameters::getParameter(const string & name) const {
		return params.const_get(name).getValue();
	}
    string & UPnPActionParameters::operator[] (const string & name) {
        return params[name];
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