#ifndef __UPNP_ACTION_RESPONSE_HPP__
#define __UPNP_ACTION_RESPONSE_HPP__

#include <map>
#include <string>
#include "UPnPModels.hpp"

namespace UPNP {

	/**
	 * @brief 
	 */
	class UPnPActionResponse {
	private:
		int _errorCode;
		std::string _serviceType;
		std::string _actionName;
		std::map<std::string, std::string> _params;
		
	public:
		UPnPActionResponse();
		virtual ~UPnPActionResponse();

		int errorCode();
		std::string & serviceType();
		std::string & actionName();
		std::map<std::string, std::string> & parameters();
		std::string & operator[] (const std::string & name);
	};
}

#endif
