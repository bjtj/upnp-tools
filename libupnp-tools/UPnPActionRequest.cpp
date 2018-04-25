#include "UPnPActionRequest.hpp"

namespace upnp {

	using namespace std;
	
	UPnPActionRequest::UPnPActionRequest() {
	}

	
	UPnPActionRequest::~UPnPActionRequest() {
	}

	string & UPnPActionRequest::serviceType() {
		return _serviceType;
	}

	UPnPAction & UPnPActionRequest::action() {
		return _action;
	}

	string & UPnPActionRequest::actionName() {
		return _action.name();
	}

	map<string, string> & UPnPActionRequest::parameters() {
		return _params;
	}

	string & UPnPActionRequest::operator[] (const string & name) {
		return _params[name];
	}

}
