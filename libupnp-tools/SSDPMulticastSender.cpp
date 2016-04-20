#include "SSDPMulticastSender.hpp"

namespace SSDP {

	using namespace std;
	using namespace OS;
	using namespace UTIL;
	
	SSDPMulticastSender::SSDPMulticastSender() {
		init();
	}
	SSDPMulticastSender::SSDPMulticastSender(int port) : sock(port) {
		init();
	}
	SSDPMulticastSender::SSDPMulticastSender(OS::InetAddress & bindAddr) : sock(bindAddr) {
		init();
	}
	SSDPMulticastSender::~SSDPMulticastSender() {
	}
	void SSDPMulticastSender::init() {
		sock.registerSelector(selector, Selector::READ);
	}
	void SSDPMulticastSender::close() {
		sock.unregisterSelector(selector, Selector::READ);
		sock.close();
	}
	void SSDPMulticastSender::poll(unsigned long timeout) {
		if (selector.select(timeout) > 0) {
			if (sock.isReadableSelected(selector)) {
				char buffer[4096] = {0,};
				DatagramPacket packet(buffer, sizeof(buffer));
				sock.recv(packet);
				onReceive(packet);
			}
		}
	}
	void SSDPMulticastSender::onReceive(OS::DatagramPacket & packet) {
		SSDPHeader header(packet.getData(), packet.getRemoteAddr());
		for (vector<AutoRef<SSDPEventHandler> >::iterator iter = handlers.begin(); iter != handlers.end(); iter++) {
			(*iter)->dispatch(header);
		}
	}
	void SSDPMulticastSender::sendMcast(const string & content, const string & group, int port) {
		char buffer[4096] = {0,};
		DatagramPacket packet(buffer, sizeof(buffer), group, port);
		packet.write(content);
		sock.send(packet);
	}
	void SSDPMulticastSender::sendMcastToAllInterfaces(const string & content, const string & group, int port) {
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
	void SSDPMulticastSender::addSSDPEventHandler(AutoRef<SSDPEventHandler> handler) {
		handlers.push_back(handler);
	}

	void SSDPMulticastSender::removeSSDPEventHandler(AutoRef<SSDPEventHandler> handler) {
		for (vector<AutoRef<SSDPEventHandler> >::iterator iter = handlers.begin(); iter != handlers.end();) {
			if ((*iter) == handler) {
				iter = handlers.erase(iter);
			} else {
				iter++;
			}
		}
	}
}
