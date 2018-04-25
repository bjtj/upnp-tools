#ifndef __UPNP_ACTION_REQUEST_HPP__
#define __UPNP_ACTION_REQUEST_HPP__

#include "UPnPModels.hpp"

namespace upnp {

	/**
	 * @brief 
	 */
	class UPnPActionRequest {
	private:
		std::string _serviceType;
		UPnPAction _action;
		std::map<std::string, std::string> _params;
		
	public:
		UPnPActionRequest();
		virtual ~UPnPActionRequest();
		std::string & serviceType();
		UPnPAction & action();
		std::string & actionName();
		std::map<std::string, std::string> & parameters();
		std::string & operator[] (const std::string & name);
	};
}

#endif
