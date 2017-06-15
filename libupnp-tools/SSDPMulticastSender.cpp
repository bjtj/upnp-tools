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
		return sock.isReadable(*selector);
	}
	bool SSDPMulticastSender::isReadable(Selector & selector) {
		return sock.isReadable(selector);
	}
	void SSDPMulticastSender::procRead() {
		DatagramPacket packet(4096);
		sock.recv(packet);
		onReceive(packet);
	}
	void SSDPMulticastSender::onReceive(DatagramPacket & packet) {
		SSDPHeader header(packet.getData(), packet.getRemoteAddr());
		for (vector<AutoRef<SSDPEventListener> >::iterator iter = listeners.begin(); iter != listeners.end(); iter++) {
			(*iter)->dispatch(header);
		}
	}
	void SSDPMulticastSender::sendMcast(const string & content, const string & group, int port) {
		DatagramPacket packet(4096, group, port);
		packet.write(content);
		sock.send(packet);
	}
	void SSDPMulticastSender::sendMcastToAllInterfaces(const string & content, const string & group, int port) {
		DatagramPacket packet(4096, group, port);
		packet.write(content);
		std::vector<InetAddress> addrs = Network::getAllInetAddress();
		for (size_t i = 0; i < addrs.size(); i++) {
			if (addrs[i].inet4()) {
				sock.setMulticastInterface(addrs[i].getHost());
				sock.send(packet);
			}
		}
	}
	void SSDPMulticastSender::addSSDPEventListener(AutoRef<SSDPEventListener> listener) {
		listeners.push_back(listener);
	}

	void SSDPMulticastSender::removeSSDPEventListener(AutoRef<SSDPEventListener> listener) {
		for (vector<AutoRef<SSDPEventListener> >::iterator iter = listeners.begin(); iter != listeners.end();) {
			if ((*iter) == listener) {
				iter = listeners.erase(iter);
			} else {
				iter++;
			}
		}
	}
}
