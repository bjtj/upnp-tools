#ifndef __SSDP_MULTICAST_LISTENER_HPP__
#define __SSDP_MULTICAST_LISTENER_HPP__

#include <string>
#include <liboslayer/os.hpp>
#include <liboslayer/DatagramSocket.hpp>
#include "SSDPEventHandler.hpp"

namespace SSDP {

	class SSDPMulticastListener {
	private:
		std::string group;
		OS::MulticastSocket sock;
		OS::Selector selector;
		SSDPEventHandler * handler;

	public:
		SSDPMulticastListener(const std::string & group, int port);
		virtual ~SSDPMulticastListener();

		void start();
		void stop();
		bool isRunning();
		void poll(unsigned long timeout);
		void onReceive(OS::DatagramPacket & packet);
		void setSSDPEventHandler(SSDPEventHandler * handler);
	};
}

#endif
