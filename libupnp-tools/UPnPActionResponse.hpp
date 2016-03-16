#ifndef __UPNP_ACTION_RESPONSE_HPP__
#define __UPNP_ACTION_RESPONSE_HPP__

#include <map>
#include <string>

namespace UPNP {
	
	class UPnPActionResponse {
	private:
		int _errorCode;
		std::map<std::string, std::string> _params;
		
	public:
		UPnPActionResponse();
		virtual ~UPnPActionResponse();

		int errorCode();
		std::map<std::string, std::string> & parameters();
		std::string & operator[] (const std::string & name);
	};
}

#endif
