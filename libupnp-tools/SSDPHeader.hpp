#ifndef __SSDP_HEADER_HPP__
#define __SSDP_HEADER_HPP__

#include <liboslayer/os.hpp>
#include <liboslayer/Network.hpp>
#include <libhttp-server/HttpHeader.hpp>
#include <libhttp-server/HttpHeaderReader.hpp>
#include "UPnPUsn.hpp"

namespace ssdp {

	class SSDPHeader : public http::HttpHeader {
	private:
		std::string rawPacket;
		osl::InetAddress remoteAddr;
	public:
		SSDPHeader();
		SSDPHeader(const std::string & headerString, osl::InetAddress & remoteAddr);
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
		std::string getNotificationType() const;
		std::string getNotificationSubType() const;
		std::string getSearchTarget() const;
		std::string getLocation() const;
		std::string getMan() const;
		osl::InetAddress getRemoteAddr() const;
		std::string getRawPacket() const;
		std::string getCacheControl() const;
		void setCacheControl(int maxAge);
		void setNotificationType(const std::string & nt);
		void setNotificationSubType(const std::string & nts);
		void setSearchTarget(const std::string & st);
		void setLocation(const std::string & location);
		void setUsn(const upnp::USN & usn);
	};
}

#endif
