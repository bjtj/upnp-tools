#include "UPnPService.hpp"

namespace UPNP {

	using namespace std;

	Scpd::Scpd() {
	}
	Scpd::~Scpd() {
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

	vector<UPnPStateVariable> & Scpd::getStateVariables() {
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



	UPnPService::UPnPService() {
	}
	
	UPnPService::~UPnPService() {
	}
	
	bool UPnPService::empty() {
		return getServiceType().empty();
	}

	void UPnPService::setServiceType(const std::string & serviceType) {
		properties["serviceType"] = serviceType;
	}
    
    string UPnPService::getServiceType() const {
        return properties["serviceType"];
    }

	void UPnPService::setScpd(const Scpd & scpd) {
		this->scpd = scpd;
	}
	Scpd & UPnPService::getScpd() {
		return scpd;
	}
    
    void UPnPService::setBaseUrl(const string & baseUrl) {
        this->baseUrl = baseUrl;
    }
    string UPnPService::getBaseUrl() const {
        return baseUrl;
    }

	string & UPnPService::operator[](const string & name) const {
		return properties[name];
	}
}
