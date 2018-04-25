#include "UPnPActionResponse.hpp"
#include "UPnPActionErrorCodes.hpp"

namespace upnp {

	using namespace std;
	using namespace osl;

	
	UPnPActionResponse::UPnPActionResponse() : _errorCode(0) {
	}
	
	UPnPActionResponse::~UPnPActionResponse() {
	}

	void UPnPActionResponse::setError(int errorCode) {
		setError(errorCode, UPnPActionErrorCodes::getDescription(errorCode));
	}
	
	void UPnPActionResponse::setError(int errorCode, const string & errorString) {
		_errorCode = errorCode;
		_errorString = errorString;
	}

	int & UPnPActionResponse::errorCode() {
		return _errorCode;
	}
	
	string & UPnPActionResponse::errorString() {
		return _errorString;
	}

	string & UPnPActionResponse::serviceType() {
		return _serviceType;
	}

	string & UPnPActionResponse::actionName() {
		return _actionName;
	}

	LinkedStringMap & UPnPActionResponse::parameters() {
		return _params;
	}

	string & UPnPActionResponse::operator[] (const string & name) {
		return _params[name];
	}
}

