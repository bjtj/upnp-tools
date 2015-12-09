#include "SSDPMsearchSender.hpp"
#include <liboslayer/Text.hpp>

namespace SSDP {

	SSDPMsearchSender::SSDPMsearchSender() : handler(NULL), _cancel(false) {
		init();
	}
	SSDPMsearchSender::SSDPMsearchSender(int port) : handler(NULL), _cancel(false), sock(port) {
		init();
	}
	SSDPMsearchSender::SSDPMsearchSender(OS::InetAddress & bindAddr) : handler(NULL), _cancel(false), sock(bindAddr) {
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
		unsigned long startTick = OS::tick_milli();
		while (!_cancel && (OS::tick_milli() - startTick < timeout)) {
			poll(100);
		}
	}
	void SSDPMsearchSender::poll(unsigned long timeout) {
		if (selector.select(timeout) > 0) {
			if (sock.isReadalbeSelected(selector)) {
				char buffer[4096] = {0,};
				OS::DatagramPacket packet(buffer, sizeof(buffer));
				sock.recv(packet);
				onReceive(packet);
			}
		}
	}
	void SSDPMsearchSender::onReceive(OS::DatagramPacket & packet) {
		SSDPHeader header(packet.getData(), packet.getRemoteAddr());

		if (header.isSSDPResponse()) {
			if (handler) {
				handler->onAnyPacket(header);
				handler->onMsearchResponse(header);
			}
		}
	}

	void SSDPMsearchSender::sendMsearch(const std::string & st, unsigned long timeoutSec, const std::string & group, int port) {
		sendMcast(makeMsearchPacket(st, timeoutSec, group, port), group, port);
	}

	void SSDPMsearchSender::sendMcast(const std::string & content, const std::string & group, int port) {
		char buffer[4096] = {0,};
		OS::DatagramPacket packet(buffer, sizeof(buffer), group, port);
		packet.write(content);
		sock.send(packet);
	}

	void SSDPMsearchSender::sendMsearchAllInterfaces(const std::string & st, unsigned long timeoutSec, const std::string & group, int port) {
		sendMcastToAllInterfaces(makeMsearchPacket(st, timeoutSec, group, port), group, port);
	}

	void SSDPMsearchSender::sendMcastToAllInterfaces(const std::string & content, const std::string & group, int port) {
		char buffer[4096] = {0,};
		OS::DatagramPacket packet(buffer, sizeof(buffer), group, port);
		packet.write(content);
		std::vector<OS::InetAddress> addrs = OS::Network::getAllInetAddress();
		for (size_t i = 0; i < addrs.size(); i++) {
			sock.setMulticastInteface(addrs[i].getHost());
			sock.send(packet);
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
	void SSDPMsearchSender::setSSDPPacketHandler(SSDPPacketHandler * handler) {
		this->handler = handler;
	}
}