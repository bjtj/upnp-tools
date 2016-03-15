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
		UPnPSessionManager() {}
		virtual ~UPnPSessionManager() {}

		bool has(const std::string & udn) {
			return (sessions.find(udn) != sessions.end());
		}
		void clear() {
			sessions.clear();
		}
		UTIL::AutoRef<UPnPSession> prepareSession(const std::string & udn) {
			if (!has(udn)) {
				sessions[udn] = UTIL::AutoRef<UPnPSession>(new UPnPSession(udn));
			}
			return sessions[udn];
		}
		void remove(const std::string & udn) {
			sessions.erase(udn);
		}
		size_t size() {
			return sessions.size();
		}
		std::vector<std::string> getUdnS() {
			std::vector<std::string> ret;
			for (std::map<std::string, UTIL::AutoRef<UPnPSession> >::iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
				ret.push_back(iter->first);
			}
			return ret;
		}
		UTIL::AutoRef<UPnPSession> operator[] (const std::string & udn) {
			return sessions[udn];
		}
	};
}

#endif
