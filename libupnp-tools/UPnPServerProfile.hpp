#ifndef __UPNP_SERVER_PROFILE_HPP__
#define __UPNP_SERVER_PROFILE_HPP__

#include <string>
#include <map>

namespace UPNP {
	
	class UPnPServerProfile {
	private:
		std::map<std::string, std::string> props;
	public:
		UPnPServerProfile();
		virtual ~UPnPServerProfile();

		std::string & operator[] (const std::string & name);
	};
}

#endif
