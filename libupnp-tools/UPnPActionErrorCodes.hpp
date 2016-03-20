#ifndef __UPNP_ACTION_ERROR_CODES_HPP__
#define __UPNP_ACTION_ERROR_CODES_HPP__

#include <string>
#include <map>

namespace UPNP {
	
	class UPnPActionErrorCodes {
	private:
		static std::map<int, std::string> errorCodes;
	public:
		UPnPActionErrorCodes();
		virtual ~UPnPActionErrorCodes();
		static std::string getDescription(int code);
	};

	
}

#endif
