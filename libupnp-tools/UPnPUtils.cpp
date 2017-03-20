#include "UPnPUtils.hpp"
#include "UPnPExceptions.hpp"
#include <liboslayer/Text.hpp>

namespace UPNP {

	using namespace std;
	using namespace UTIL;

	MaxAge::MaxAge(const string & phrase) {
		parse(phrase);
	}
	MaxAge::MaxAge(unsigned long second) : _second(second) {
	}
	MaxAge::~MaxAge() {
	}
	unsigned long & MaxAge::second() {
		return _second;
	}
	void MaxAge::parse(const string & phrase) {
		if (Text::startsWithIgnoreCase(phrase, "max-age=") == false) {
			throw UPnPParseException("max-age not occurred");
		}
		_second = (unsigned long)Text::toLong(phrase.substr(string("max-age=").size()));
	}
	string MaxAge::toString() {
		return "max-age=" + Text::toString(_second);
	}
	string MaxAge::toString(unsigned long second) {
		return "max-age=" + Text::toString(second);
	}

}
