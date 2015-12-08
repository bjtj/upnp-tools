#include "SSDPMulticastListener.hpp"

namespace SSDP {
	
	SSDPMulticastListener::SSDPMulticastListener(const std::string & group, int port) : group(group), sock(port), handler(NULL) {
	}
	SSDPMulticastListener::~SSDPMulticastListener() {
	}
	void SSDPMulticastListener::start() {
		sock.joinGroup(group);
		sock.registerSelector(selector);
	}
	void SSDPMulticastListener::stop() {
		sock.unregisterSelector(selector);
		sock.close();
	}
	void SSDPMulticastListener::poll(unsigned long timeout) {
		if (selector.select(timeout) > 0) {
			if (sock.isReadalbeSelected(selector)) {
				char buffer[4096] = {0,};
				OS::DatagramPacket packet(buffer, sizeof(buffer));
				sock.recv(packet);
				onReceive(packet);
			}
		}
	}
	void SSDPMulticastListener::onReceive(OS::DatagramPacket & packet) {
		SSDPHeader header(packet.getData(), packet.getRemoteAddr());

		if (handler) {
			handler->onAnyPacket(header);
			if (header.isMsearch()) {
				handler->onMsearch(header);
			} else if (header.isNotify()) {
				handler->onNotify(header);
			}
		}
	}
	void SSDPMulticastListener::setSSDPPacketHandler(SSDPPacketHandler * listener) {
		this->handler = listener;
	}
}