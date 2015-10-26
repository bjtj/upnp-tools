#include "UPnPService.hpp"

namespace UPNP {

	using namespace std;

	UPnPService::UPnPService() {
	}
	
	UPnPService::~UPnPService() {
	}
	
	string UPnPService::getServiceType() {
		return serviceType;
	}
	
	UPnPAction UPnPService::getAction(std::string name) {
		for (vector<UPnPAction>::iterator iter = actions.begin();
			 iter != actions.end(); iter++) {

			UPnPAction & action = *iter;
			if (!action.getName().compare(name)) {
				return action;
			}
		}
		return UPnPAction();
	}

	vector<UPnPAction> & UPnPService::getActions() {
		return actions;
	}

	vector<UPnPStateVariable> & UPnPService::getStateVariables() {
		return stateVariables;
	}

	bool UPnPService::isEmpty() {
		return serviceType.empty();
	}

	void UPnPService::setServiceType(const std::string & serviceType) {
		this->serviceType = serviceType;
	}

	void UPnPService::setActions(std::vector<UPnPAction> & actions) {
		this->actions = actions;
	}

	void UPnPService::setStateVariables(std::vector<UPnPStateVariable> & stateVariables) {
		this->stateVariables = stateVariables;
	}

	bool UPnPService::operator==(const UPnPService &other) const {
		return (this->serviceType.compare(other.serviceType) == 0);
	}

	string & UPnPService::operator[](const string & name) {
		return properties[name];
	}
}
