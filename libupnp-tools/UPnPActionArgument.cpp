#include "UPnPActionArgument.hpp"
#include <liboslayer/Text.hpp>

namespace UPNP {

	using namespace std;
    using namespace UTIL;
	
	UPnPActionArgument::UPnPActionArgument() {
	}
	UPnPActionArgument::~UPnPActionArgument() {
	}

	bool UPnPActionArgument::in() const {
        return Text::equalsIgnoreCase(direction, "in");
	}
	bool UPnPActionArgument::out() const {
        return Text::equalsIgnoreCase(direction, "out");
	}
	string UPnPActionArgument::getName() const {
		return name;
	}
	string UPnPActionArgument::getStateVariableName() const {
		return stateVariable.getName();
	}
    const UPnPStateVariable & UPnPActionArgument::getStateVariable() const {
        return stateVariable;
    }
    UPnPStateVariable & UPnPActionArgument::getStateVariable() {
        return stateVariable;
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
