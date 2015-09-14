#include "UPnPService.hpp"

namespace UPNP {

	using namespace std;

	UPnPService::UPnPService() {
	}

	UPnPService::UPnPService(std::string serviceType) : serviceType(serviceType) {
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

	bool UPnPService::isEmpty() {
		return serviceType.empty();
	}

	bool UPnPService::operator==(const UPnPService &other) const {
		return (this->serviceType.compare(other.serviceType) == 0);
	}
}
