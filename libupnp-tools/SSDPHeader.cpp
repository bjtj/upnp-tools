#include "SSDPHeader.hpp"
#include <liboslayer/Text.hpp>

namespace ssdp {

	using namespace std;
	using namespace osl;


	SSDPHeader::SSDPHeader() {
	}

	SSDPHeader::SSDPHeader(const string & headerString, osl::InetAddress & remoteAddr)
		: remoteAddr(remoteAddr) {
		http::HttpHeaderReader reader;
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
			Text::equalsIgnoreCase(getNotificationSubType(), "ssdp:alive")) {
			return true;
		}
		return false;
	}

	bool SSDPHeader::isNotifyByebye() const {
		if (Text::equalsIgnoreCase(getMethod(), "NOTIFY") &&
			Text::equalsIgnoreCase(getNotificationSubType(), "ssdp:byebye")) {
			return true;
		}
		return false;
	}

	bool SSDPHeader::isNotifyUpdate() const {
		if (Text::equalsIgnoreCase(getMethod(), "NOTIFY") &&
			Text::equalsIgnoreCase(getNotificationSubType(), "ssdp:update")) {
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
		return getHeaderField("USN");
	}

	string SSDPHeader::getNotificationType() const {
		return getHeaderField("NT");
	}

	string SSDPHeader::getNotificationSubType() const {
		return getHeaderField("NTS");
	}

	string SSDPHeader::getSearchTarget() const {
		return getHeaderField("ST");
	}

	string SSDPHeader::getLocation() const {
		return getHeaderField("LOCATION");
	}

	string SSDPHeader::getMan() const {
		return getHeaderField("MAN");
	}

	osl::InetAddress SSDPHeader::getRemoteAddr() const {
		return remoteAddr;
	}

	string SSDPHeader::getRawPacket() const {
		return rawPacket;
	}

	string SSDPHeader::getCacheControl() const {
		return getHeaderField("Cache-Control");
	}

	void SSDPHeader::setCacheControl(int maxAge) {
		setHeaderField("Cache-Control", Text::toString(maxAge));
	}

	void SSDPHeader::setNotificationType(const string & nt) {
		setHeaderField("NT", nt);
	}

	void SSDPHeader::setNotificationSubType(const string & nts) {
		setHeaderField("NTS", nts);
	}

	void SSDPHeader::setSearchTarget(const string & st) {
		setHeaderField("ST", st);
	}

	void SSDPHeader::setLocation(const string & location) {
		setHeaderField("LOCATION", location);
	}

	void SSDPHeader::setUsn(const upnp::USN & usn) {
		setHeaderField("USN", usn.toString());
	}

}
