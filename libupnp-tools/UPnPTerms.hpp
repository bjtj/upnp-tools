#ifndef __UPNP_TERMS_HPP__
#define __UPNP_TERMS_HPP__

#include <string>
#include <vector>

namespace upnp {

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
		explicit UDN(const std::string & str);
		virtual ~UDN();
		std::string & str();
		std::string str() const;
		std::string uuid() const;
		bool empty() const;
		bool valid() const;
		std::string toString() const;
		bool operator< (const UDN & other) const;
		bool operator< (const std::string & str) const;
		bool operator> (const UDN & other) const;
		bool operator> (const std::string & str) const;
		bool operator== (const UDN & other) const;
		bool operator== (const std::string & str) const;
	};

	/**
	 * max age
	 */
	class MaxAge {
	private:
		unsigned long _second;
	public:
		MaxAge(unsigned long second);
		virtual ~MaxAge();
		unsigned long & second();
		static MaxAge fromString(const std::string & phrase);
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
		CallbackUrls(const std::vector<std::string> & urls);
		virtual ~CallbackUrls();
		std::vector<std::string> & urls();
		static CallbackUrls fromString(const std::string & phrase);
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
		Second(unsigned long second);
		virtual ~Second();
		unsigned long & second();
		unsigned long second() const;
		unsigned long milli() const;
		static Second fromString(const std::string & phrase);
		std::string toString() const;
		static std::string toString(unsigned long second);
	};
}

#endif
