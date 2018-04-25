#ifndef __UPNP_CONFIG_HPP__
#define __UPNP_CONFIG_HPP__

#include <string>


namespace upnp {

	class UPnPConfig {
	private:
		std::string _user_agent;
		static UPnPConfig _instance;
	private:
		UPnPConfig();
	public:
		virtual ~UPnPConfig();
		static UPnPConfig & instance();
		std::string & user_agent();
	};
}

#endif
