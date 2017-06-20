#include "UPnPTerms.hpp"
#include "UPnPExceptions.hpp"
#include <liboslayer/Text.hpp>
#include <libhttp-server/Url.hpp>

namespace UPNP {

	using namespace std;
	using namespace UTIL;
	using namespace HTTP;

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
		if (valid() == false) {
			return "";
		}
		size_t s = _str.find("::");
		if (s != string::npos) {
			return _str.substr(5, s - 5);
		}
		return _str.substr(5);
	}
	string USN::rest() const {
		size_t s = _str.find("::");
		if (s != string::npos) {
			return _str.substr(s + 2);
		}
		return "";
	}
	bool USN::empty() const {
		return _str.empty();
	}
	bool USN::valid() const {
		if (Text::startsWith(_str, "uuid:") == false) {
			return false;
		}
		return true;
	}
	string USN::toString() const {
		return _str;
	}


	/**
	 * udn
	 */

	UDN::UDN() {
	}
	UDN::UDN(const string & str) : _str(str) {
	}
	UDN::~UDN() {
	}
	string & UDN::str() {
		return _str;
	}
	string UDN::str() const {
		return _str;
	}
	string UDN::uuid() const {
		if (valid() == false) {
			return "";
		}
		return _str.substr(5);
	}
	bool UDN::empty() const {
		return _str.empty();
	}
	bool UDN::valid() const {
		if (Text::startsWith(_str, "uuid:") == false) {
			return false;
		}
		return true;
	}
	string UDN::toString() const {
		return _str;
	}

	/**
	 * max-age
	 */

	MaxAge::MaxAge(const string & phrase) : _second(parse(phrase)) {
	}
	MaxAge::MaxAge(unsigned long second) : _second(second) {
	}
	MaxAge::~MaxAge() {
	}
	unsigned long & MaxAge::second() {
		return _second;
	}
	unsigned long MaxAge::parse(const string & phrase) {
		if (Text::startsWithIgnoreCase(phrase, "max-age=") == false) {
			throw UPnPParseException("max-age not occurred");
		}
		return (unsigned long)Text::toLong(phrase.substr(string("max-age=").size()));
	}
	string MaxAge::toString() const {
		return toString(_second);
	}
	string MaxAge::toString(unsigned long second) {
		return "max-age=" + Text::toString(second);
	}

	/**
	 * callback urls
	 */
	
	CallbackUrls::CallbackUrls(const string & phrase) : _urls(parse(phrase)) {
	}
	
	CallbackUrls::CallbackUrls(const vector<string> & urls) : _urls(urls) {
	}
	
	CallbackUrls::~CallbackUrls() {
	}
	
	vector<string> & CallbackUrls::urls() {
		return _urls;
	}
	
	vector<string> CallbackUrls::parse(const string & phrase) {
		vector<string> urls;
		string buffer;
		if (phrase.empty()) {
			throw UPnPParseException("empty string");
		}
		for (string::const_iterator iter = phrase.begin(); iter != phrase.end(); iter++) {
			if (*iter == '<') {
				buffer = "";
				for (iter++; iter != phrase.end() && *iter != '>'; iter++) {
					buffer.append(1, *iter);
				}
				try {
					Url::validateUrlFormat(buffer);
				} catch (UrlParseException e) {
					throw UPnPParseException("wrong url format - '" + buffer + "'");
				}
				urls.push_back(buffer);
			}
		}
		return urls;
	}
	
	string CallbackUrls::toString() const {
		return toString(_urls);
	}
	
	string CallbackUrls::toString(const vector<string> & urls) {
		string ret;
		for (size_t i = 0; i < urls.size(); i++) {
			if (i != 0) {
				ret.append(" ");
			}
			ret.append("<" + urls[i] + ">");
		}
		return ret;
	}

	/**
	 * second
	 */
	Second::Second(const string & phrase) : _second(parse(phrase)) {
	}
	
	Second::Second(unsigned long second) : _second(second) {
	}
	
	Second::~Second() {
	}
	
	unsigned long & Second::second() {
		return _second;
	}
	
	unsigned long Second::parse(const string & phrase) {
		if (Text::startsWithIgnoreCase(phrase, "Second-") == false) {
			throw UPnPParseException("Not found prefix 'Second-'");
		}
		return (unsigned long)Text::toLong(phrase.substr(string("Second-").size()));
	}

	string Second::toString() const {
		return toString(_second);
	}
	
	string Second::toString(unsigned long second) {
		return "Second-" + Text::toString(second);
	}
}
