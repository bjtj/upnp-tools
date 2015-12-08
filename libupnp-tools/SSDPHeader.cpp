#include "SSDPHeader.hpp"

#include <liboslayer/Text.hpp>

namespace SSDP {
	
	SSDPHeader::SSDPHeader(const std::string & headerString, OS::InetAddress & remoteAddr) : remoteAddr(remoteAddr) {
		reader.read(headerString.c_str(), headerString.length());
		this->setHeader(reader.getHeader());
	}
	SSDPHeader::~SSDPHeader() {
	}
	bool SSDPHeader::isSSDPRequest() {
		if (UTIL::Text::equalsIgnoreCase(getMethod(), "NOTIFY") || UTIL::Text::equalsIgnoreCase(getMethod(), "M-SEARCH")) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isSSDPResponse() {
		if (UTIL::Text::startsWith(getMethod(), "HTTP/", true)) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isNotifyAlive() {
		if (UTIL::Text::equalsIgnoreCase(getMethod(), "NOTIFY") && UTIL::Text::equalsIgnoreCase(getNts(), "ssdp:alive")) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isNotifyByebye() {
		if (UTIL::Text::equalsIgnoreCase(getMethod(), "NOTIFY") && UTIL::Text::equalsIgnoreCase(getNts(), "ssdp:byebye")) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isNotifyUpdate() {
		if (UTIL::Text::equalsIgnoreCase(getMethod(), "NOTIFY") && UTIL::Text::equalsIgnoreCase(getNts(), "ssdp:update")) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isNotify() {
		if (UTIL::Text::equalsIgnoreCase(getMethod(), "NOTIFY")) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isMsearch() {
		if (UTIL::Text::equalsIgnoreCase(getMethod(), "M-SEARCH")) {
			return true;
		}
		return false;
	}
	std::string SSDPHeader::getMethod() {
		return getPart1();
	}
	std::string SSDPHeader::getUsn() {
		return getHeaderFieldIgnoreCase("USN");
	}
	std::string SSDPHeader::getNt() {
		return getHeaderFieldIgnoreCase("NT");
	}
	std::string SSDPHeader::getNts() {
		return getHeaderFieldIgnoreCase("NTS");
	}
	std::string SSDPHeader::getSt() {
		return getHeaderFieldIgnoreCase("ST");
	}
	std::string SSDPHeader::getLocation() {
		return getHeaderFieldIgnoreCase("LOCATION");
	}
	std::string SSDPHeader::getMan() {
		return getHeaderFieldIgnoreCase("MAN");
	}
	OS::InetAddress SSDPHeader::getRemoteAddr() {
		return remoteAddr;
	}
}