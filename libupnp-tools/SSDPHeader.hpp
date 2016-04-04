#ifndef __SSDP_HEADER_HPP__
#define __SSDP_HEADER_HPP__

#include <liboslayer/os.hpp>
#include <libhttp-server/HttpHeader.hpp>
#include <libhttp-server/HttpHeaderReader.hpp>

namespace SSDP {

	class SSDPHeader : public HTTP::HttpHeader {
	private:
		std::string rawPacket;
		OS::InetAddress remoteAddr;
	public:
		SSDPHeader(const std::string & headerString, OS::InetAddress & remoteAddr);
		virtual ~SSDPHeader();
		bool isSSDPRequest() const;
		bool isSSDPResponse() const;
		bool isNotifyAlive() const;
		bool isNotifyByebye() const;
		bool isNotifyUpdate() const;
		bool isNotify() const;
		bool isMsearch() const;
		std::string getMethod() const;
		std::string getUsn() const;
		std::string getNt() const;
		std::string getNts() const;
		std::string getSt() const;
		std::string getLocation() const;
		std::string getMan() const;
		OS::InetAddress getRemoteAddr() const;
		std::string getRawPacket() const;
		std::string getCacheControl() const;
	};

}

#endif
