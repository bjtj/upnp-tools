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
	
	string UPnPService::getServiceType() {
		return properties["serviceType"];
	}
	

	bool UPnPService::empty() {
		return getServiceType().empty();
	}

	void UPnPService::setServiceType(const std::string & serviceType) {
		properties["serviceType"] = serviceType;
	}

	void UPnPService::setScpd(const Scpd & scpd) {
		this->scpd = scpd;
	}
	Scpd & UPnPService::getScpd() {
		return scpd;
	}

	string & UPnPService::operator[](const string & name) {
		return properties[name];
	}
}
