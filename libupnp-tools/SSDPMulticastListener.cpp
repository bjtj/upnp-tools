#include "SSDPMulticastListener.hpp"

namespace SSDP {

	using namespace std;
	using namespace OS;
	
	SSDPMulticastListener::SSDPMulticastListener(const string & group, int port) : group(group), sock(port) {
	}
	SSDPMulticastListener::~SSDPMulticastListener() {
	}
	void SSDPMulticastListener::start() {
		sock.joinGroup(group);
		sock.registerSelector(selector, Selector::READ);
	}
	void SSDPMulticastListener::stop() {
		sock.unregisterSelector(selector, Selector::READ);
		sock.close();
	}
	bool SSDPMulticastListener::isRunning() {
		return !sock.isClosed();
	}
	void SSDPMulticastListener::poll(unsigned long timeout) {
		if (selector.select(timeout) > 0) {
			if (sock.isReadableSelected(selector)) {
				char buffer[4096] = {0,};
				OS::DatagramPacket packet(buffer, sizeof(buffer));
				sock.recv(packet);
				onReceive(packet);
			}
		}
	}
	void SSDPMulticastListener::onReceive(OS::DatagramPacket & packet) {
		SSDPHeader header(packet.getData(), packet.getRemoteAddr());

		if (!handler.nil() && handler->filter(header)) {
			if (header.isMsearch()) {
				handler->onMsearch(header);
			} else if (header.isNotify()) {
				handler->onNotify(header);
			}
		}
	}
	void SSDPMulticastListener::setSSDPEventHandler(UTIL::AutoRef<SSDPEventHandler> handler) {
		this->handler = handler;
	}
}
