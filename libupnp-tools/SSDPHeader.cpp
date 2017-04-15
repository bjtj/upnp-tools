#include "SSDPHeader.hpp"
#include <liboslayer/Text.hpp>

namespace SSDP {

	using namespace std;
	using namespace UTIL;
	
	SSDPHeader::SSDPHeader(const string & headerString, OS::InetAddress & remoteAddr)
		: remoteAddr(remoteAddr) {
		HTTP::HttpHeaderReader reader;
		reader.read(headerString.c_str(), headerString.length());
		this->setHeader(reader.getHeader());
		rawPacket = headerString;
	}
	SSDPHeader::~SSDPHeader() {
	}
	bool SSDPHeader::isSSDPRequest() const {
		if (Text::equalsIgnoreCase(getMethod(), "NOTIFY") ||
			Text::equalsIgnoreCase(getMethod(), "M-SEARCH")) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isSSDPResponse() const {
		if (Text::startsWith(getMethod(), "HTTP/", true)) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isNotifyAlive() const {
		if (Text::equalsIgnoreCase(getMethod(), "NOTIFY") &&
			Text::equalsIgnoreCase(getNts(), "ssdp:alive")) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isNotifyByebye() const {
		if (Text::equalsIgnoreCase(getMethod(), "NOTIFY") &&
			Text::equalsIgnoreCase(getNts(), "ssdp:byebye")) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isNotifyUpdate() const {
		if (Text::equalsIgnoreCase(getMethod(), "NOTIFY") &&
			Text::equalsIgnoreCase(getNts(), "ssdp:update")) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isNotify() const {
		if (Text::equalsIgnoreCase(getMethod(), "NOTIFY")) {
			return true;
		}
		return false;
	}
	bool SSDPHeader::isMsearch() const {
		if (Text::equalsIgnoreCase(getMethod(), "M-SEARCH")) {
			return true;
		}
		return false;
	}
	string SSDPHeader::getMethod() const {
		return getPart1();
	}
	string SSDPHeader::getUsn() const {
		return getHeaderFieldIgnoreCase("USN");
	}
	string SSDPHeader::getNt() const {
		return getHeaderFieldIgnoreCase("NT");
	}
	string SSDPHeader::getNts() const {
		return getHeaderFieldIgnoreCase("NTS");
	}
	string SSDPHeader::getSt() const {
		return getHeaderFieldIgnoreCase("ST");
	}
	string SSDPHeader::getLocation() const {
		return getHeaderFieldIgnoreCase("LOCATION");
	}
	string SSDPHeader::getMan() const {
		return getHeaderFieldIgnoreCase("MAN");
	}
	OS::InetAddress SSDPHeader::getRemoteAddr() const {
		return remoteAddr;
	}
	string SSDPHeader::getRawPacket() const {
		return rawPacket;
	}
	string SSDPHeader::getCacheControl() const {
		return getHeaderFieldIgnoreCase("Cache-Control");
	}
}
