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

class SSDPMsearchSender {
private:
	DatagramSocket sock;
	Selector selector;
	SSDPPacketListener * listener;
	bool _cancel;
	int port;
public:
	SSDPMsearchSender() : listener(NULL), _cancel(false), port(0) {
	}
	SSDPMsearchSender(int port) : listener(NULL), _cancel(false), port(port) {
	}
	virtual ~SSDPMsearchSender() {
	}
	void start() {
		sock.bind(Inet4Address(port));
		sock.registerSelector(selector);
	}
	void stop() {
		sock.unregisterSelector(selector);
		sock.close();
	}
	bool isRunning() {
		return sock.isClosed() == false;
	}
	void cancel() {
		_cancel = true;
	}
	void gather(unsigned long timeout) {
		unsigned long startTick = tick_milli();
		while (!_cancel && (tick_milli() - startTick < timeout)) {
			poll(100);
		}
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

		if (header.isSSDPResponse()) {
			if (listener) {
				listener->onAnyPacket(header);
				listener->onResponse(header);
			}
		}
	}

	void sendMsearch(const std::string & st, unsigned long timeoutSec, const std::string & group, int port) {

		std::string msearchPacket = "M-SEARCH * HTTP/1.1\r\n"
			"HOST: " + group + ":" + Text::toString(port) + "\r\n"
			"MAN: \"ssdp:discover\"\r\n"
			"MX: " + Text::toString(timeoutSec) + "\r\n"
			"ST: " + st + "\r\n"
			"USER-AGENT: OS/version UPnP/1.1 product/version\r\n"
			"\r\n";

		sendMcast(msearchPacket, group, port);
	}

	void sendMcast(const std::string & content, const std::string & group, int port) {
		char buffer[4096] = {0,};
		DatagramPacket packet(buffer, sizeof(buffer), group, port);
		packet.write(content);
		sock.send(packet);
	}
	InetAddress getLocalInetAddress() {
		return sock.getLocalInetAddress();
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

class SSDPMsearchSenderFactory {
private:
	std::map<int, SSDPMsearchSender*> persistences;
	std::map<int, SSDPMsearchSender*> instances;
public:
	SSDPMsearchSenderFactory() {
	}
	virtual ~SSDPMsearchSenderFactory() {
		clearAll();
	}
	void clear(std::map<int, SSDPMsearchSender*> & senders) {
		for (std::map<int, SSDPMsearchSender*>::iterator iter = senders.begin(); iter != senders.end(); iter++) {
			iter->second->stop();
			delete iter->second;
			iter->second = NULL;
		}
	}
	void clearAll() {
		clear(persistences);
		clear(instances);
	}
	SSDPMsearchSender * getPersistentSender(int port) {
		if (persistences.find(port) != persistences.end()) {
			return persistences[port];
		}
		SSDPMsearchSender * sender = new SSDPMsearchSender(port);
		sender->start();
		int boundPort = sender->getLocalInetAddress().getPort();
		persistences[boundPort] = sender;
		return sender;
	}
	SSDPMsearchSender * getInstantSender(int port) {
		if (instances.find(port) != instances.end()) {
			return instances[port];
		}
		SSDPMsearchSender * sender = new SSDPMsearchSender(port);
		sender->start();
		int boundPort = sender->getLocalInetAddress().getPort();
		instances[boundPort] = sender;
		return sender;
	}
	void release(SSDPMsearchSender * sender) {
		int port = sender->getLocalInetAddress().getPort();
		if (instances.find(port) != instances.end()) {
			sender->stop();
			delete sender;
			instances.erase(port);
		}
		if (persistences.find(port) != persistences.end() && sender->isRunning() == false) {
			delete sender;
			persistences.erase(port);
		}
	}
};

static void s_test_msearch(const std::string & st) {

	MySSDPPacketListener packetListener;
	SSDPMsearchSender sender;
	sender.setSSDPPacketListener(&packetListener);

	sender.start();

	for (int i = 0; i < 3; i++) {
		sender.sendMsearch(st, 3, "239.255.255.250", 1900);
		sender.gather(3000);
	}

	sender.stop();
}

static void s_test_msearch_with_factory(const std::string & st) {

	SSDPMsearchSenderFactory factory;
	SSDPMsearchSender * sender = factory.getPersistentSender(0);

	MySSDPPacketListener packetListener;
	sender->setSSDPPacketListener(&packetListener);

	printf("BOUND PORT: %d\n", sender->getLocalInetAddress().getPort());

	for (int i = 0; i < 3; i++) {
		sender->sendMsearch(st, 3, "239.255.255.250", 1900);
		sender->gather(3000);
	}

	factory.release(sender);
}

static void s_test_msearch_with_factory_instance(const std::string & st) {

	SSDPMsearchSenderFactory factory;

	for (int i = 0; i < 3; i++) {

		SSDPMsearchSender * sender = factory.getInstantSender(0);

		MySSDPPacketListener packetListener;
		sender->setSSDPPacketListener(&packetListener);

		printf("BOUND PORT: %d\n", sender->getLocalInetAddress().getPort());

		sender->sendMsearch(st, 3, "239.255.255.250", 1900);
		idle(300);
		sender->sendMsearch(st, 3, "239.255.255.250", 1900);
		idle(300);
		sender->sendMsearch(st, 3, "239.255.255.250", 1900);
		sender->gather(3000);
	
		factory.release(sender);

	}
}

int main(int argc, char * args[]) {

	//s_test_msearch("upnp:rootdevice");
	s_test_msearch_with_factory("upnp:rootdevice");
	//s_test_msearch_with_factory_instance("upnp:rootdevice");

	printf("done\n");
	getchar();

	return 0;
}