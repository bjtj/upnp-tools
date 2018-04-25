#include "SSDPMsearchSender.hpp"
#include <liboslayer/Text.hpp>
#include "UPnPConfig.hpp"

namespace ssdp {

	using namespace std;
	using namespace osl;

	SSDPMsearchSender::SSDPMsearchSender()
		: _cancel(false) {
	}

	SSDPMsearchSender::SSDPMsearchSender(AutoRef<Selector> selector)
		: SSDPMulticastSender(selector), _cancel(false) {
	}

	SSDPMsearchSender::SSDPMsearchSender(int port)
		: SSDPMulticastSender(port), _cancel(false) {
	}

	SSDPMsearchSender::SSDPMsearchSender(int port, AutoRef<Selector> selector)
		: SSDPMulticastSender(port, selector), _cancel(false) {
	}

	SSDPMsearchSender::SSDPMsearchSender(InetAddress & bindAddr)
		: SSDPMulticastSender(bindAddr), _cancel(false) {
	}

	SSDPMsearchSender::SSDPMsearchSender(InetAddress & bindAddr, AutoRef<Selector> selector)
		: SSDPMulticastSender(bindAddr, selector), _cancel(false) {
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

	void SSDPMsearchSender::sendMsearch(const string & st, unsigned long timeoutSec, const string & group, int port) {
		sendMcast(makeMsearchPacket(st, timeoutSec, group, port), group, port);
	}

	void SSDPMsearchSender::sendMsearchAllInterfaces(const string & st, unsigned long timeoutSec, const string & group, int port) {
		sendMcastToAllInterfaces(makeMsearchPacket(st, timeoutSec, group, port), group, port);
	}

	string SSDPMsearchSender::makeMsearchPacket(const string & st, unsigned long timeoutSec, const string & group, int port) {
		return "M-SEARCH * HTTP/1.1\r\n"
			"HOST: " + group + ":" + Text::toString(port) + "\r\n"
			"MAN: \"ssdp:discover\"\r\n"
			"MX: " + Text::toString(timeoutSec) + "\r\n"
			"ST: " + st + "\r\n"
			"USER-AGENT: " + upnp::UPnPConfig::instance().user_agent() + "\r\n"
			"\r\n";
	}

	void SSDPMsearchSender::unicast(const string & content, InetAddress & remoteAddr) {
		DatagramSocket sock;
		DatagramPacket packet(content.size(), remoteAddr);
		packet.write(content);
		sock.send(packet);
	}

}
