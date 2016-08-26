#include "SSDPMulticastSender.hpp"

namespace SSDP {

	using namespace std;
	using namespace OS;
	using namespace UTIL;
	
	SSDPMulticastSender::SSDPMulticastSender() : selector(new Selector) {
		init();
	}
	SSDPMulticastSender::SSDPMulticastSender(AutoRef<Selector> selector) : selector(selector) {
		init();
	}
	SSDPMulticastSender::SSDPMulticastSender(int port) : sock(port), selector(new Selector) {
		init();
	}
	SSDPMulticastSender::SSDPMulticastSender(int port, AutoRef<Selector> selector) : sock(port), selector(selector) {
		init();
	}
	SSDPMulticastSender::SSDPMulticastSender(InetAddress & bindAddr) : sock(bindAddr), selector(new Selector) {
		init();
	}
	SSDPMulticastSender::SSDPMulticastSender(InetAddress & bindAddr, AutoRef<Selector> selector) : sock(bindAddr), selector(selector) {
		init();
	}
	SSDPMulticastSender::~SSDPMulticastSender() {
	}
	void SSDPMulticastSender::init() {
		sock.registerSelector(*selector, Selector::READ);
	}
	void SSDPMulticastSender::close() {
		sock.unregisterSelector(*selector, Selector::READ);
		sock.close();
	}
	void SSDPMulticastSender::poll(unsigned long timeout) {
		if (selector->select(timeout) > 0) {
			if (isReadable()) {
				procRead();
			}
		}
	}
	bool SSDPMulticastSender::isReadable() {
		return sock.isReadableSelected(*selector);
	}
	bool SSDPMulticastSender::isReadable(Selector & selector) {
		return sock.isReadableSelected(selector);
	}
	void SSDPMulticastSender::procRead() {
		char buffer[4096] = {0,};
		DatagramPacket packet(buffer, sizeof(buffer));
		sock.recv(packet);
		onReceive(packet);
	}
	void SSDPMulticastSender::onReceive(DatagramPacket & packet) {
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
