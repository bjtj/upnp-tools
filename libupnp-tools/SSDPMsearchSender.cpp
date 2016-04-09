#include "SSDPMsearchSender.hpp"
#include <liboslayer/Text.hpp>

namespace SSDP {

	using namespace std;
	using namespace OS;

	SSDPMsearchSender::SSDPMsearchSender() : _cancel(false) {
		init();
	}
	SSDPMsearchSender::SSDPMsearchSender(int port) : sock(port), _cancel(false) {
		init();
	}
	SSDPMsearchSender::SSDPMsearchSender(InetAddress & bindAddr) : sock(bindAddr), _cancel(false) {
		init();

	}
	SSDPMsearchSender::~SSDPMsearchSender() {
	}
	void SSDPMsearchSender::init() {
		sock.registerSelector(selector);
	}
	void SSDPMsearchSender::close() {
		cancel();
		sock.unregisterSelector(selector);
		sock.close();
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
	void SSDPMsearchSender::poll(unsigned long timeout) {
		if (selector.select(timeout) > 0) {
			if (sock.isReadalbeSelected(selector)) {
				char buffer[4096] = {0,};
				DatagramPacket packet(buffer, sizeof(buffer));
				sock.recv(packet);
				onReceive(packet);
			}
		}
	}
	void SSDPMsearchSender::onReceive(DatagramPacket & packet) {
		SSDPHeader header(packet.getData(), packet.getRemoteAddr());

		if (header.isSSDPResponse()) {
			if (!handler.nil() && handler->filter(header)) {
				handler->onMsearchResponse(header);
			}
		}
	}

	void SSDPMsearchSender::sendMsearch(const std::string & st, unsigned long timeoutSec, const std::string & group, int port) {
		sendMcast(makeMsearchPacket(st, timeoutSec, group, port), group, port);
	}

	void SSDPMsearchSender::sendMcast(const std::string & content, const std::string & group, int port) {
		char buffer[4096] = {0,};
		DatagramPacket packet(buffer, sizeof(buffer), group, port);
		packet.write(content);
		sock.send(packet);
	}

	void SSDPMsearchSender::sendMsearchAllInterfaces(const std::string & st, unsigned long timeoutSec, const std::string & group, int port) {
		sendMcastToAllInterfaces(makeMsearchPacket(st, timeoutSec, group, port), group, port);
	}

	void SSDPMsearchSender::sendMcastToAllInterfaces(const std::string & content, const std::string & group, int port) {
		char buffer[4096] = {0,};
		DatagramPacket packet(buffer, sizeof(buffer), group, port);
		packet.write(content);
		std::vector<InetAddress> addrs = Network::getAllInetAddress();
		for (size_t i = 0; i < addrs.size(); i++) {
			if (addrs[i].inet4()) {
				sock.setMulticastInterface(addrs[i].getHost());
				sock.send(packet);
			}
		}
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
	void SSDPMsearchSender::setSSDPEventHandler(UTIL::AutoRef<SSDPEventHandler> handler) {
		this->handler = handler;
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
