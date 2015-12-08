#ifndef __SSDP_MULTICAST_LISTENER_HPP__
#define __SSDP_MULTICAST_LISTENER_HPP__

#include <string>
#include <liboslayer/os.hpp>
#include <liboslayer/DatagramSocket.hpp>
#include "AnotherSSDPServer.hpp"
#include "SSDPPacketHandler.hpp"

namespace SSDP {

	class SSDPMulticastListener {
	private:
		std::string group;
		OS::MulticastSocket sock;
		OS::Selector selector;
		SSDPPacketHandler * handler;

	public:
		SSDPMulticastListener(const std::string & group, int port);
		virtual ~SSDPMulticastListener();

		void start();
		void stop();
		void poll(unsigned long timeout);
		void onReceive(OS::DatagramPacket & packet);
		void setSSDPPacketHandler(SSDPPacketHandler * listener);
	};
}

#endif