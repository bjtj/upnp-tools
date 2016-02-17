#include "SSDPHeader.hpp"

#include <liboslayer/Text.hpp>

namespace SSDP {
	
	SSDPHeader::SSDPHeader(const std::string & headerString, OS::InetAddress & remoteAddr) : remoteAddr(remoteAddr) {
		reader.read(headerString.c_str(), headerString.length());
		this->setHeader(reader.getHeader());
	}
	SSDPHeader::~SSDPHeader() {
	}
	bool SSDPHeader::isSSDPRequest() const {
		if (UTIL::Text::equalsIgnoreCase(getMethod(), "NOTIFY") || UTIL::Text::equalsIgnoreCase(getMethod(), "M-SEARCH")) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isSSDPResponse() const {
		if (UTIL::Text::startsWith(getMethod(), "HTTP/", true)) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isNotifyAlive() const {
		if (UTIL::Text::equalsIgnoreCase(getMethod(), "NOTIFY") && UTIL::Text::equalsIgnoreCase(getNts(), "ssdp:alive")) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isNotifyByebye() const {
		if (UTIL::Text::equalsIgnoreCase(getMethod(), "NOTIFY") && UTIL::Text::equalsIgnoreCase(getNts(), "ssdp:byebye")) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isNotifyUpdate() const {
		if (UTIL::Text::equalsIgnoreCase(getMethod(), "NOTIFY") && UTIL::Text::equalsIgnoreCase(getNts(), "ssdp:update")) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isNotify() const {
		if (UTIL::Text::equalsIgnoreCase(getMethod(), "NOTIFY")) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isMsearch() const {
		if (UTIL::Text::equalsIgnoreCase(getMethod(), "M-SEARCH")) {
			return true;
		}
		return false;
	}
	std::string SSDPHeader::getMethod() const {
		return getPart1();
	}
	std::string SSDPHeader::getUsn() const {
		return getHeaderFieldIgnoreCase("USN");
	}
	std::string SSDPHeader::getNt() const {
		return getHeaderFieldIgnoreCase("NT");
	}
	std::string SSDPHeader::getNts() const {
		return getHeaderFieldIgnoreCase("NTS");
	}
	std::string SSDPHeader::getSt() const {
		return getHeaderFieldIgnoreCase("ST");
	}
	std::string SSDPHeader::getLocation() const {
		return getHeaderFieldIgnoreCase("LOCATION");
	}
	std::string SSDPHeader::getMan() const {
		return getHeaderFieldIgnoreCase("MAN");
	}
	OS::InetAddress SSDPHeader::getRemoteAddr() const {
		return remoteAddr;
	}
}
