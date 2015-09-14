#include "UPnPAction.hpp"

namespace UPNP {
	
	using namespace std;

	UPnPAction::UPnPAction() {
	}
	UPnPAction::~UPnPAction() {
	}
	
	string UPnPAction::getName() {
		return name;
	}
	vector<UPnPActionArgument> & UPnPAction::getArguments() {
		return arguments;
	}
	vector<UPnPStateVariable> & UPnPAction::getStateVariables() {
		return stateVariable;
	}
}
