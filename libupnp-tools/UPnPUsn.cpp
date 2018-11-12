#include "UPnPUsn.hpp"

namespace upnp {

	using namespace std;

	/**
	 * usn
	 */

	USN::USN() {
	}

	USN::USN(const string & str) : _str(str) {
	}

	USN::~USN() {
	}

	string & USN::str() {
		return _str;
	}

	string USN::str() const {
		return _str;
	}

	string USN::uuid() const {
		size_t s = _str.find("::");
		if (s != string::npos) {
			return _str.substr(0, s);
		}
		return _str;
	}

	string USN::type() const {
		size_t s = _str.find("::");
		if (s != string::npos) {
			return _str.substr(s + 2);
		}
		return "";
	}

	bool USN::empty() const {
		return _str.empty();
	}

	string USN::toString() const {
		return _str;
	}

}
