#ifndef __SSDP_MULTICAST_LISTENER_HPP__
#define __SSDP_MULTICAST_LISTENER_HPP__

#include <string>
#include <liboslayer/os.hpp>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/DatagramSocket.hpp>
#include "SSDPEventHandler.hpp"

namespace SSDP {

	class SSDPMulticastListener {
	private:
		std::string group;
		OS::MulticastSocket sock;
		UTIL::AutoRef<OS::Selector> selector;
		std::vector<UTIL::AutoRef<SSDPEventHandler> > handlers;

	public:
		SSDPMulticastListener(const std::string & group, int port);
		SSDPMulticastListener(const std::string & group, int port, UTIL::AutoRef<OS::Selector> selector);
		virtual ~SSDPMulticastListener();

		void start();
		void stop();
		bool isRunning();
		void poll(unsigned long timeout);
		bool isReadable();
		bool isReadable(OS::Selector & selector);
		void procRead();
		void onReceive(OS::DatagramPacket & packet);
		void addSSDPEventHandler(UTIL::AutoRef<SSDPEventHandler> handler);
		void removeSSDPEventHandler(UTIL::AutoRef<SSDPEventHandler> handler);
	};
}

#endif
