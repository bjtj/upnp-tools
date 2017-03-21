#include "UPnPUtils.hpp"
#include "UPnPExceptions.hpp"
#include <liboslayer/Text.hpp>
#include <libhttp-server/Url.hpp>

namespace UPNP {

	using namespace std;
	using namespace UTIL;
	using namespace HTTP;


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
	string MaxAge::toString() {
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
				} catch (UrlParseException & e) {
					throw UPnPParseException("wrong url format - '" + buffer + "'");
				}
				urls.push_back(buffer);
			}
		}
		return urls;
	}
	
	string CallbackUrls::toString() {
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

	std::string Second::toString() {
		return toString(_second);
	}
	
	std::string Second::toString(unsigned long second) {
		return "Second-" + Text::toString(second);
	}
}
