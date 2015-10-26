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

	void UPnPAction::setArguments(vector<UPnPActionArgument> & arguments) {
		this->arguments = arguments;
	}

	bool UPnPAction::isEmpty() {
		return name.empty();
	}
}
