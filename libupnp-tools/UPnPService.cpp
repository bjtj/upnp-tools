#include "UPnPService.hpp"

namespace UPNP {

	using namespace std;
    using namespace UTIL;
    
	Scpd::Scpd() {
	}
	Scpd::~Scpd() {
	}

	void Scpd::clear() {
		actions.clear();
		stateVariables.clear();
		properties.clear();
	}

	UPnPAction Scpd::getAction(string name) {
		for (vector<UPnPAction>::iterator iter = actions.begin();
			 iter != actions.end(); iter++) {

			UPnPAction & action = *iter;
			if (!action.getName().compare(name)) {
				return action;
			}
		}
		return UPnPAction();
	}

	vector<UPnPAction> & Scpd::getActions() {
		return actions;
	}
    
    const vector<UPnPAction> & Scpd::getActions() const {
        return actions;
    }

	vector<UPnPStateVariable> & Scpd::getStateVariables() {
		return stateVariables;
	}
    
    const vector<UPnPStateVariable> & Scpd::getStateVariables() const {
        return stateVariables;
    }

	void Scpd::setActions(vector<UPnPAction> & actions) {
		this->actions = actions;
	}

	void Scpd::setStateVariables(vector<UPnPStateVariable> & stateVariables) {
		this->stateVariables = stateVariables;
	}

	string & Scpd::operator[](const string & name) {
		return properties[name];
	}


    /**
     * @brief
     */

	UPnPService::UPnPService() : scpdBind(false) {
	}
	
	UPnPService::~UPnPService() {
	}
	
	bool UPnPService::empty() {
		return getServiceType().empty();
	}
    
    void UPnPService::clear() {
        properties.clear();
		scpd.clear();
    }

	void UPnPService::setServiceType(const std::string & serviceType) {
		properties["serviceType"] = serviceType;
	}
    
    string UPnPService::getServiceType() const {
		return properties.const_get("serviceType").getValue();
    }

	void UPnPService::setScpd(const Scpd & scpd) {
		scpdBind = true;
		this->scpd = scpd;
	}
	Scpd & UPnPService::getScpd() {
		return scpd;
	}

	const Scpd & UPnPService::getScpd() const {
		return scpd;
	}
    
    void UPnPService::setBaseUrl(const string & baseUrl) {
        this->baseUrl = baseUrl;
    }
    string UPnPService::getBaseUrl() const {
        return baseUrl;
    }

	bool UPnPService::isScpdBind() const {
		return scpdBind;
	}
    
	string UPnPService::getProperty(const string & name) const {
		return properties.const_get(name).getValue();
	}
    LinkedStringMap & UPnPService::getProperties() {
        return properties;
    }
    const LinkedStringMap & UPnPService::getProperties() const {
        return properties;
    }
    
    string UPnPService::getScpdUrl() {
        return properties["SCPDURL"];
    }
    string UPnPService::getControlUrl() {
        return properties["controlURL"];
    }
    string UPnPService::getEventSubscribeUrl() {
        return properties["eventSubURL"];
    }

    string & UPnPService::operator[](const string & name) {
        return properties[name];
    }
}
