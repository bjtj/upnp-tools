#include "UPnPActionErrorCodes.hpp"

namespace upnp {

	using namespace std;

	static map<int, string> createCodes();

	map<int, string> UPnPActionErrorCodes::errorCodes = createCodes();
	
	UPnPActionErrorCodes::UPnPActionErrorCodes() {
	}

	UPnPActionErrorCodes::~UPnPActionErrorCodes() {
	}

	string UPnPActionErrorCodes::getDescription(int code) {
		return errorCodes[code];
	}

	static map<int, string> createCodes() {
		map<int, string> codes;
		codes[401] = "Invalid Action";
		codes[402] = "Invalid Args";
		codes[501] = "Action Failed";
		codes[600] = "Argument Value Invalid";
		codes[601] = "Argument Value Out of Range";
		codes[602] = "Optional Action Not Implemented";
		codes[603] = "Out of Memory";
		codes[604] = "Human Intervention Required";
		codes[605] = "String Argument Too Long";
		return codes;
	}

}
