#ifndef __SSDP_MULTICAST_SENDER_HPP__
#define __SSDP_MULTICAST_SENDER_HPP__

#include <vector>
#include <string>
#include "SSDPEventHandler.hpp"
#include <liboslayer/os.hpp>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/DatagramSocket.hpp>

namespace SSDP {
	
	class SSDPMulticastSender {
	private:
		OS::DatagramSocket sock;
		UTIL::AutoRef<OS::Selector> selector;
		std::vector<UTIL::AutoRef<SSDPEventHandler> > handlers;
	public:
		SSDPMulticastSender();
		SSDPMulticastSender(UTIL::AutoRef<OS::Selector> selector);
		SSDPMulticastSender(int port);
		SSDPMulticastSender(int port, UTIL::AutoRef<OS::Selector> selector);
		SSDPMulticastSender(OS::InetAddress & bindAddr);
		SSDPMulticastSender(OS::InetAddress & bindAddr, UTIL::AutoRef<OS::Selector> selector);
		virtual ~SSDPMulticastSender();
		void init();
		void close();
		void poll(unsigned long timeout);
		bool isReadable();
		bool isReadable(OS::Selector & selector);
		void procRead();
		void onReceive(OS::DatagramPacket & packet);
		void sendMcast(const std::string & content, const std::string & group, int port);
		void sendMcastToAllInterfaces(const std::string & content, const std::string & group, int port);
		void addSSDPEventHandler(UTIL::AutoRef<SSDPEventHandler> handler);
		void removeSSDPEventHandler(UTIL::AutoRef<SSDPEventHandler> handler);
	};
}

#endif
