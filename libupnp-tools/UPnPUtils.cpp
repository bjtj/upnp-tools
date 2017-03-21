#include "UPnPUtils.hpp"
#include "UPnPExceptions.hpp"
#include <liboslayer/Text.hpp>
#include <libhttp-server/Url.hpp>

namespace UPNP {

	using namespace std;
	using namespace UTIL;
	using namespace HTTP;


	/**
	 * 
	 */

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

	/**
	 * callback urls
	 */
	
	CallbackUrls::CallbackUrls(const string & phrase) {
		parse(phrase);
	}
	CallbackUrls::CallbackUrls(const std::vector<std::string> & urls) : _urls(urls) {
	}
	CallbackUrls::~CallbackUrls() {
	}
	vector<string> & CallbackUrls::urls() {
		return _urls;
	}
	void CallbackUrls::parse(const string & phrase) {
		string buffer;
		_urls.clear();
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
				_urls.push_back(buffer);
			}
		}
	}
	string CallbackUrls::toString() {
		string ret;
		for (size_t i = 0; i < _urls.size(); i++) {
			if (i != 0) {
				ret.append(" ");
			}
			ret.append("<" + _urls[i] + ">");
		}
		return ret;
	}
}
