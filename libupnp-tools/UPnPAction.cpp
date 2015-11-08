#include "UPnPAction.hpp"

namespace UPNP {
	
	using namespace std;

	UPnPAction::UPnPAction() {
	}
	UPnPAction::~UPnPAction() {
	}
    
    void UPnPAction::setName(const string & name) {
        this->name = name;
    }
	
	string UPnPAction::getName() const {
		return name;
	}
	vector<UPnPActionArgument> & UPnPAction::getArguments() {
		return arguments;
	}
    
    const vector<UPnPActionArgument> & UPnPAction::getArguments() const {
        return arguments;
    }

	void UPnPAction::setArguments(const vector<UPnPActionArgument> & arguments) {
		this->arguments = arguments;
	}

	bool UPnPAction::isEmpty() {
		return name.empty();
	}
}
