#include "UPnPActionArgument.hpp"

namespace UPNP {
	
	UPnPActionArgument::UPnPActionArgument() : direction(UNKNOWN_DIRECTION) {
	}
	UPnPActionArgument::~UPnPActionArgument() {
	}

	bool UPnPActionArgument::inArgument() {
		return direction == IN_DIRECTION;
	}
	bool UPnPActionArgument::outArgument() {
		return direction == OUT_DIRECTION;
	}
	string UPnPActionArgument::getName() {
		return name;
	}
	string UPnPActionArgument::getStateVariableName() {
		return stateVariable.getName();
	}
	
}
