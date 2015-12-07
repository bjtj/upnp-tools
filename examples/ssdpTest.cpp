#include <liboslayer/os.hpp>
#include <liboslayer/DatagramSocket.hpp>
#include <liboslayer/Text.hpp>
#include <libhttp-server/HttpHeader.hpp>
#include <libhttp-server/HttpHeaderReader.hpp>

using namespace OS;
using namespace HTTP;
using namespace UTIL;

class SSDPHeader : public HttpHeader {
private:
	HttpHeaderReader reader;
	InetAddress remoteAddr;
public:
	SSDPHeader(const std::string & headerString, InetAddress & remoteAddr) : remoteAddr(remoteAddr) {
		reader.read(headerString.c_str(), headerString.length());
		this->setHeader(reader.getHeader());
	}
	virtual ~SSDPHeader() {
	}
	bool isSSDPRequest() {
		if (Text::equalsIgnoreCase(getMethod(), "NOTIFY") || Text::equalsIgnoreCase(getMethod(), "M-SEARCH")) {
			return true;
		}
		return false;
	}
	bool isSSDPResponse() {
		if (Text::startsWith(getMethod(), "HTTP/", true)) {
			return true;
		}
		return false;
	}
	bool isNotifyAlive() {
		if (Text::equalsIgnoreCase(getMethod(), "NOTIFY") && Text::equalsIgnoreCase(getNts(), "ssdp:alive")) {
			return true;
		}
		return false;
	}
	bool isNotifyByebye() {
		if (Text::equalsIgnoreCase(getMethod(), "NOTIFY") && Text::equalsIgnoreCase(getNts(), "ssdp:byebye")) {
			return true;
		}
		return false;
	}
	bool isNotifyUpdate() {
		if (Text::equalsIgnoreCase(getMethod(), "NOTIFY") && Text::equalsIgnoreCase(getNts(), "ssdp:update")) {
			return true;
		}
		return false;
	}
	bool isNotify() {
		if (Text::equalsIgnoreCase(getMethod(), "NOTIFY")) {
			return true;
		}
		return false;
	}
	bool isMsearch() {
		if (Text::equalsIgnoreCase(getMethod(), "M-SEARCH")) {
			return true;
		}
		return false;
	}
	std::string getMethod() {
		return getPart1();
	}
	std::string getUsn() {
		return getHeaderFieldIgnoreCase("USN");
	}
	std::string getNt() {
		return getHeaderFieldIgnoreCase("NT");
	}
	std::string getNts() {
		return getHeaderFieldIgnoreCase("NTS");
	}
	std::string getSt() {
		return getHeaderFieldIgnoreCase("ST");
	}
	std::string getLocation() {
		return getHeaderFieldIgnoreCase("LOCATION");
	}
	std::string getMan() {
		return getHeaderFieldIgnoreCase("MAN");
	}
	InetAddress getRemoteAddr() {
		return remoteAddr;
	}
};

class SSDPPacketListener {
private:
public:
	SSDPPacketListener() {}
	virtual ~SSDPPacketListener() {}

	virtual void onAnyPacket(SSDPHeader & header) {}
	virtual void onMsearch(SSDPHeader & header) {}
	virtual void onNotify(SSDPHeader & header) {}
	virtual void onResponse(SSDPHeader & header) {}
};

class SSDPMutlicastListener {
private:
	std::string group;
	MulticastSocket sock;
	Selector selector;
	SSDPPacketListener * listener;
public:
	SSDPMutlicastListener(const std::string & group, int port) : group(group), sock(port), listener(NULL) {
	}
	virtual ~SSDPMutlicastListener() {
	}
	void start() {
		sock.joinGroup(group);
		sock.registerSelector(selector);
	}
	void stop() {
		sock.unregisterSelector(selector);
		sock.close();
	}
	void poll(unsigned long timeout) {
		if (selector.select(timeout) > 0) {
			if (sock.isReadalbeSelected(selector)) {
				char buffer[4096] = {0,};
				DatagramPacket packet(buffer, sizeof(buffer));
				sock.recv(packet);
				onReceive(packet);
			}
		}
	}
	void onReceive(DatagramPacket & packet) {
		SSDPHeader header(packet.getData(), packet.getRemoteAddr());

		if (listener) {
			listener->onAnyPacket(header);
			if (header.isMsearch()) {
				listener->onMsearch(header);
			} else if (header.isNotify()) {
				listener->onNotify(header);
			}
		}
	}
	void setSSDPPacketListener(SSDPPacketListener * listener) {
		this->listener = listener;
	}
};

class MySSDPPacketListener : public SSDPPacketListener {
private:
public:
	MySSDPPacketListener() {}
	virtual ~MySSDPPacketListener() {}
	virtual void onAnyPacket(SSDPHeader & header) {
		// printf("FROM %s:%d\n%s", header.getRemoteAddr().getHost().c_str(), header.getRemoteAddr().getPort(), header.toString().c_str());
	}
	virtual void onMsearch(SSDPHeader & header) {
		printf("onMsearch :: %s (%s:%d)\n", header.getSt().c_str(), header.getRemoteAddr().getHost().c_str(), header.getRemoteAddr().getPort());
	}
	virtual void onNotify(SSDPHeader & header) {
		printf("onNotify :: %s (%s:%d)\n", header.getLocation().c_str(), header.getRemoteAddr().getHost().c_str(), header.getRemoteAddr().getPort());
	}
	virtual void onResponse(SSDPHeader & header) {
		printf("onResponse :: %s (%s:%d)\n", header.getLocation().c_str(), header.getRemoteAddr().getHost().c_str(), header.getRemoteAddr().getPort());
	}
};

static void s_test_listener() {

	MySSDPPacketListener packetListener;
	SSDPMutlicastListener ssdpListener("239.255.255.250", 1900);
	ssdpListener.setSSDPPacketListener(&packetListener);
	ssdpListener.start();

	while (1) {
		ssdpListener.poll(100);
	}

	ssdpListener.stop();

}

int main(int argc, char * args[]) {

	s_test_listener();

	return 0;
}