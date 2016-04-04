#include "UPnPSessionManager.hpp"

namespace UPNP {

	using namespace std;
	using namespace UTIL;
	
	UPnPSessionManager::UPnPSessionManager() {}
	UPnPSessionManager::~UPnPSessionManager() {}

	bool UPnPSessionManager::has(const string & udn) {
		return (sessions.find(udn) != sessions.end());
	}
	void UPnPSessionManager::clear() {
		sessions.clear();
	}
	AutoRef<UPnPSession> UPnPSessionManager::prepareSession(const string & udn) {
		if (!has(udn)) {
			sessions[udn] = AutoRef<UPnPSession>(new UPnPSession(udn));
		}
		return sessions[udn];
	}
	void UPnPSessionManager::remove(const string & udn) {
		sessions.erase(udn);
	}
	size_t UPnPSessionManager::size() {
		return sessions.size();
	}
	vector<string> UPnPSessionManager::getUdnS() {
		vector<string> ret;
		for (map<string, AutoRef<UPnPSession> >::iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
			ret.push_back(iter->first);
		}
		return ret;
	}
	AutoRef<UPnPSession> UPnPSessionManager::operator[] (const string & udn) {
		return sessions[udn];
	}
	vector<AutoRef<UPnPSession> > UPnPSessionManager::getOutdatedSessions() {
		vector<AutoRef<UPnPSession> > ret;
		for (map<string, AutoRef<UPnPSession> >::iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
			if (iter->second->outdated()) {
				ret.push_back(iter->second);
			}
		}
		return ret;
	}
}

