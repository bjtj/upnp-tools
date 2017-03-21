#ifndef __UPNP_UTILS_HPP__
#define __UPNP_UTILS_HPP__

#include <string>
#include <vector>

namespace UPNP {

	/**
	 * max age
	 */
	class MaxAge {
	private:
		unsigned long _second;
	public:
		MaxAge(const std::string & phrase);
		MaxAge(unsigned long second);
		virtual ~MaxAge();
		unsigned long & second();
		void parse(const std::string & phrase);
		std::string toString();
		static std::string toString(unsigned long second);
	};

	/**
	 * callback urls
	 */
	class CallbackUrls {
	private:
		std::vector<std::string> _urls;
	public:
		CallbackUrls(const std::string & phrase);
		CallbackUrls(const std::vector<std::string> & urls);
		virtual ~CallbackUrls();
		std::vector<std::string> & urls();
		void parse(const std::string & phrase);
		std::string toString();
	};

}

#endif
