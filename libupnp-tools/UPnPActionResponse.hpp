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
		std::string _errorString;
		std::string _serviceType;
		std::string _actionName;
		UTIL::LinkedStringMap _params;
		
	public:
		UPnPActionResponse();
		virtual ~UPnPActionResponse();

		void setError(int errorCode);
		void setError(int errorCode, const std::string & errorString);
		int & errorCode();
		std::string & errorString();
		std::string & serviceType();
		std::string & actionName();
		UTIL::LinkedStringMap & parameters();
		std::string & operator[] (const std::string & name);
	};
}

#endif
