#ifndef __UPNP_UTILS_HPP__
#define __UPNP_UTILS_HPP__

#include <string>

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
}

#endif
