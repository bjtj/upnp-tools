#include "UPnPConfig.hpp"

namespace UPNP {


	UPnPConfig UPnPConfig::_instance;
	
	UPnPConfig::UPnPConfig()
		: _user_agent("OS/x.x UPnP/1.1 App/x.x")
	{
	}
	
	UPnPConfig::~UPnPConfig() {
	}
	
	UPnPConfig & UPnPConfig::instance() {
		return _instance;
	}
	
	std::string & UPnPConfig::user_agent() {
		return _user_agent;
	}

}
