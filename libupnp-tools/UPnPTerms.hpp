#ifndef __UPNP_UTILS_HPP__
#define __UPNP_UTILS_HPP__

#include <string>
#include <vector>

namespace UPNP {

	/**
	 * USN (Unique Service Name)
	 */
	class USN {
	private:
		std::string _str;
	public:
		USN();
		USN(const std::string & str);
		virtual ~USN();
		std::string & str();
		std::string str() const;
		std::string uuid() const;
		std::string rest() const;
		bool empty() const;
		bool valid() const;
		std::string toString() const;
	};

	/**
	 * UDN (Unique Device Name)
	 */
	class UDN {
	private:
		std::string _str;
	public:
		UDN();
		UDN(const std::string & str);
		virtual ~UDN();
		std::string & str();
		std::string str() const;
		std::string uuid() const;
		bool empty() const;
		bool valid() const;
		std::string toString() const;
	};

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
		static unsigned long parse(const std::string & phrase);
		std::string toString() const;
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
		static std::vector<std::string> parse(const std::string & phrase);
		std::string toString() const;
		static std::string toString(const std::vector<std::string> & _urls);
	};

	/**
	 * second
	 */
	class Second {
	private:
		unsigned long _second;
	public:
		Second(const std::string & phrase);
		Second(unsigned long second);
		virtual ~Second();
		unsigned long & second();
		static unsigned long parse(const std::string & phrase);
		std::string toString() const;
		static std::string toString(unsigned long second);
	};
}

#endif
