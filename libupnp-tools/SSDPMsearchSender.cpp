#include "SSDPMsearchSender.hpp"
#include <liboslayer/Text.hpp>

namespace SSDP {

	using namespace std;
	using namespace OS;

	SSDPMsearchSender::SSDPMsearchSender() : _cancel(false) {
	}
	SSDPMsearchSender::SSDPMsearchSender(int port) : SSDPMulticastSender(port), _cancel(false) {
	}
	SSDPMsearchSender::SSDPMsearchSender(InetAddress & bindAddr) : SSDPMulticastSender(bindAddr), _cancel(false) {
	}
	SSDPMsearchSender::~SSDPMsearchSender() {
	}

	void SSDPMsearchSender::cancel() {
		_cancel = true;
	}
	
	void SSDPMsearchSender::gather(unsigned long timeout) {
		unsigned long startTick = tick_milli();
		while (!_cancel && (tick_milli() - startTick < timeout)) {
			poll(100);
		}
	}

	void SSDPMsearchSender::sendMsearch(const std::string & st, unsigned long timeoutSec, const std::string & group, int port) {
		sendMcast(makeMsearchPacket(st, timeoutSec, group, port), group, port);
	}

	void SSDPMsearchSender::sendMsearchAllInterfaces(const std::string & st, unsigned long timeoutSec, const std::string & group, int port) {
		sendMcastToAllInterfaces(makeMsearchPacket(st, timeoutSec, group, port), group, port);
	}

	std::string SSDPMsearchSender::makeMsearchPacket(const std::string & st, unsigned long timeoutSec, const std::string & group, int port) {
		return "M-SEARCH * HTTP/1.1\r\n"
			"HOST: " + group + ":" + UTIL::Text::toString(port) + "\r\n"
			"MAN: \"ssdp:discover\"\r\n"
			"MX: " + UTIL::Text::toString(timeoutSec) + "\r\n"
			"ST: " + st + "\r\n"
			"USER-AGENT: OS/version UPnP/1.1 product/version\r\n"
			"\r\n";
	}

	void SSDPMsearchSender::unicast(const string & content, InetAddress & remoteAddr) {
		DatagramSocket sock;
		char * buffer = new char[content.size()];
		DatagramPacket packet(buffer, content.size(), remoteAddr);
		packet.write(content);
		sock.send(packet);
		delete[] buffer;
	}
}
