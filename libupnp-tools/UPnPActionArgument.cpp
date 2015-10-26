#include "UPnPActionArgument.hpp"

namespace UPNP {

	using namespace std;
	
	UPnPActionArgument::UPnPActionArgument() {
	}
	UPnPActionArgument::~UPnPActionArgument() {
	}

	bool UPnPActionArgument::inArgument() {
		return !direction.compare("in");
	}
	bool UPnPActionArgument::outArgument() {
		return !direction.compare("out");
	}
	string UPnPActionArgument::getName() {
		return name;
	}
	string UPnPActionArgument::getStateVariableName() {
		return stateVariable.getName();
	}
	
	void UPnPActionArgument::setName(const string & name) {
		this->name = name;
	}

	void UPnPActionArgument::setStateVariable(const UPnPStateVariable & stateVariable) {
		this->stateVariable = stateVariable;
	}

	void UPnPActionArgument::setDirection(const string & direction) {
		this->direction = direction;
	}
}
