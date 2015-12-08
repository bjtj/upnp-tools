#ifndef __SSDP_HEADER_HPP__
#define __SSDP_HEADER_HPP__

#include <liboslayer/os.hpp>
#include <libhttp-server/HttpHeader.hpp>
#include <libhttp-server/HttpHeaderReader.hpp>

namespace SSDP {

	class SSDPHeader : public HTTP::HttpHeader {
	private:
		HTTP::HttpHeaderReader reader;
		OS::InetAddress remoteAddr;
	public:
		SSDPHeader(const std::string & headerString, OS::InetAddress & remoteAddr);
		virtual ~SSDPHeader();
		bool isSSDPRequest();
		bool isSSDPResponse();
		bool isNotifyAlive();
		bool isNotifyByebye();
		bool isNotifyUpdate();
		bool isNotify();
		bool isMsearch();
		std::string getMethod();
		std::string getUsn();
		std::string getNt();
		std::string getNts();
		std::string getSt();
		std::string getLocation();
		std::string getMan();
		OS::InetAddress getRemoteAddr();
	};

}

#endif