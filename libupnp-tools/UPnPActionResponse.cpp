#include "UPnPActionResponse.hpp"

namespace UPNP {

	using namespace std;
	
	UPnPActionResponse::UPnPActionResponse() {
	}
	UPnPActionResponse::~UPnPActionResponse() {
	}

	int UPnPActionResponse::errorCode() {
		return _errorCode;
	}
	string & UPnPActionResponse::serviceType() {
		return _serviceType;
	}
	string & UPnPActionResponse::actionName() {
		return _actionName;
	}
	map<string, string> & UPnPActionResponse::parameters() {
		return _params;
	}
	string & UPnPActionResponse::operator[] (const string & name) {
		return _params[name];
	}
}

