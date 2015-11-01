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
	
	string UPnPAction::getName() {
		return name;
	}
	vector<UPnPActionArgument> & UPnPAction::getArguments() {
		return arguments;
	}

	void UPnPAction::setArguments(vector<UPnPActionArgument> & arguments) {
		this->arguments = arguments;
	}

	bool UPnPAction::isEmpty() {
		return name.empty();
	}
}
