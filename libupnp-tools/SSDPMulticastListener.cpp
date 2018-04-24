#include "SSDPMulticastListener.hpp"
#include <liboslayer/File.hpp>
#include <liboslayer/Logger.hpp>

namespace SSDP {

	using namespace std;
	using namespace OS;
	using namespace UTIL;

	
    static AutoRef<Logger> logger = LoggerFactory::instance().
		getObservingLogger(File::basename(__FILE__));
	
	
	SSDPMulticastListener::SSDPMulticastListener(const string & group, int port)
		: group(group), sock(port), selector(new Selector) {
	}

	SSDPMulticastListener::SSDPMulticastListener(const std::string & group, int port, AutoRef<Selector> selector)
		: selector(selector) {
	}

	SSDPMulticastListener::~SSDPMulticastListener() {
	}

	void SSDPMulticastListener::start() {
		sock.joinGroup(group);
		sock.registerSelector(*selector, Selector::READ);
	}

	void SSDPMulticastListener::stop() {
		sock.unregisterSelector(*selector, Selector::READ);
		sock.close();
	}

	bool SSDPMulticastListener::isRunning() {
		return !sock.isClosed();
	}

	void SSDPMulticastListener::poll(unsigned long timeout) {
		if (selector->select(timeout) > 0) {
			if (isReadable()) {
				procRead();
			}
		}
	}

	bool SSDPMulticastListener::isReadable() {
		return sock.isReadable(*selector);
	}

	bool SSDPMulticastListener::isReadable(Selector & selector) {
		return sock.isReadable(selector);
	}

	void SSDPMulticastListener::procRead() {
		DatagramPacket packet(4096);
		sock.recv(packet);
		onReceive(packet);
	}

	void SSDPMulticastListener::onReceive(DatagramPacket & packet) {
		SSDPHeader header(packet.getData(), packet.getRemoteAddr());
		for (vector< AutoRef<SSDPEventListener> >::iterator iter = listeners.begin();
			 iter != listeners.end(); iter++) {
			(*iter)->dispatch(header);
		}
	}

	void SSDPMulticastListener::addSSDPEventListener(AutoRef<SSDPEventListener> listener) {
		listeners.push_back(listener);
	}

	void SSDPMulticastListener::removeSSDPEventListener(AutoRef<SSDPEventListener> listener) {
		for (vector< AutoRef<SSDPEventListener> >::iterator iter = listeners.begin(); iter != listeners.end();) {
			if ((*iter) == listener) {
				iter = listeners.erase(iter);
			} else {
				iter++;
			}
		}
	}

}
