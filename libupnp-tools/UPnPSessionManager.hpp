#ifndef __UPNP_SESSION_MANAGER_HPP__
#define __UPNP_SESSION_MANAGER_HPP__

#include "UPnPSession.hpp"

namespace UPNP {
	
	/**
	 *
	 */
	class UPnPSessionManager {
	private:
		std::map<std::string, UTIL::AutoRef<UPnPSession> > sessions;
		
	public:
		UPnPSessionManager();
		virtual ~UPnPSessionManager();
		bool has(const std::string & udn);
		void clear();
		UTIL::AutoRef<UPnPSession> prepareSession(const std::string & udn);
		void remove(const std::string & udn);
		size_t size();
		std::vector<std::string> getUdnS();
		UTIL::AutoRef<UPnPSession> operator[] (const std::string & udn);
		std::vector<UTIL::AutoRef<UPnPSession> > getOutdatedSessions();
	};
}

#endif
