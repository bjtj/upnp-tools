#include "UPnPServerProfile.hpp"

namespace UPNP {

	using namespace std;
	
	UPnPServerProfile::UPnPServerProfile() {
	}
	UPnPServerProfile::~UPnPServerProfile() {
	}

	string & UPnPServerProfile::operator[] (const string & name) {
		return props[name];
	}
}
