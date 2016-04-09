#include "UPnPDeviceProfileSessionManager.hpp"

namespace UPNP {

	using namespace std;
	using namespace UTIL;
	
	UPnPDeviceProfileSessionManager::UPnPDeviceProfileSessionManager() {
	}
	UPnPDeviceProfileSessionManager::~UPnPDeviceProfileSessionManager() {
	}
	map<string, AutoRef<UPnPDeviceProfileSession> > & UPnPDeviceProfileSessionManager::sessions() {
		return _sessions;
	}
	vector<AutoRef<UPnPDeviceProfileSession> > UPnPDeviceProfileSessionManager::searchProfileSessions(const string & st) {
		vector<AutoRef<UPnPDeviceProfileSession> > ret;
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> session = iter->second;
			if (session->profile().match(st)) {
				ret.push_back(session);
			}
		}
		return ret;
	}
	bool UPnPDeviceProfileSessionManager::hasDeviceProfileSessionWithScpdUrl(const string & scpdUrl) {
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> session = iter->second;
			if (session->profile().hasServiceWithScpdUrl(scpdUrl)) {
				return true;
			}
		}
		return false;
	}
	bool UPnPDeviceProfileSessionManager::hasDeviceProfileSessionWithControlUrl(const string & controlUrl) {
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> session = iter->second;
			if (session->profile().hasServiceWithControlUrl(controlUrl)) {
				return true;
			}
		}
		return false;
	}
	bool UPnPDeviceProfileSessionManager::hasDeviceProfileSessionWithEventSubUrl(const string & eventSubUrl) {
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> session = iter->second;
			if (session->profile().hasServiceWithEventSubUrl(eventSubUrl)) {
				return true;
			}
		}
		return false;
	}
	AutoRef<UPnPDeviceProfileSession> UPnPDeviceProfileSessionManager::getDeviceProfileSessionWithUuid(const string & uuid) {
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> session = iter->second;
			if (session->profile().uuid() == uuid) {
				return iter->second;
			}
		}
		throw OS::Exception("not found deivce profile session");
	}
	AutoRef<UPnPDeviceProfileSession> UPnPDeviceProfileSessionManager::getDeviceProfileSessionHasScpdUrl(const string & scpdUrl) {
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> session = iter->second;
			if (session->profile().hasServiceWithScpdUrl(scpdUrl)) {
				return iter->second;
			}
		}
		throw OS::Exception("not found deivce profile session");
	}
	AutoRef<UPnPDeviceProfileSession> UPnPDeviceProfileSessionManager::getDeviceProfileSessionHasEventSubUrl(const string & eventSubUrl) {
		for (map<string, AutoRef<UPnPDeviceProfileSession> >::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			AutoRef<UPnPDeviceProfileSession> session = iter->second;
			if (session->profile().hasServiceWithEventSubUrl(eventSubUrl)) {
				return iter->second;
			}
		}
		throw OS::Exception("not found deivce profile session");
	}

	AutoRef<UPnPDeviceProfileSession> UPnPDeviceProfileSessionManager::operator[] (const std::string & uuid) {
		return _sessions[uuid];
	}
}
