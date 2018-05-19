#include "UPnPPropertySet.hpp"

namespace upnp {

	using namespace std;

	/**
	 *
	 */

	UPnPPropertySet::UPnPPropertySet() : _seq(0) {
	}
	
	UPnPPropertySet::UPnPPropertySet(const string & sid) : _sid(sid), _seq(0) {
	}
	
	UPnPPropertySet::UPnPPropertySet(const string & sid, unsigned long seq) : _sid(sid), _seq(seq) {
	}
	
	UPnPPropertySet::~UPnPPropertySet() {
	}
	
	string & UPnPPropertySet::sid() {
		return _sid;
	}

	const string & UPnPPropertySet::sid() const {
		return _sid;
	}
	
	unsigned long & UPnPPropertySet::seq() {
		return _seq;
	}

	const unsigned long & UPnPPropertySet::seq() const {
		return _seq;
	}
	
	vector<string> UPnPPropertySet::propertyNames() {
		vector<string> names;
		for (map<string, string>::iterator iter = _props.begin(); iter != _props.end(); iter++) {
			names.push_back(iter->first);
		}
		return names;
	}
	
	string & UPnPPropertySet::operator[] (const string & name) {
		return _props[name];
	}
	
	UPnPEventSubscription & UPnPPropertySet::subscription() {
		return _subscription;
	}

}
