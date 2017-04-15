#ifndef __SSDP_MULTICAST_SENDER_HPP__
#define __SSDP_MULTICAST_SENDER_HPP__

#include <vector>
#include <string>
#include "SSDPEventListener.hpp"
#include <liboslayer/os.hpp>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/DatagramSocket.hpp>

namespace SSDP {
	
	class SSDPMulticastSender {
	private:
		OS::DatagramSocket sock;
		OS::AutoRef<OS::Selector> selector;
		std::vector<OS::AutoRef<SSDPEventListener> > listeners;
		
	public:
		SSDPMulticastSender();
		SSDPMulticastSender(OS::AutoRef<OS::Selector> selector);
		SSDPMulticastSender(int port);
		SSDPMulticastSender(int port, OS::AutoRef<OS::Selector> selector);
		SSDPMulticastSender(OS::InetAddress & bindAddr);
		SSDPMulticastSender(OS::InetAddress & bindAddr, OS::AutoRef<OS::Selector> selector);
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
		void addSSDPEventListener(OS::AutoRef<SSDPEventListener> listener);
		void removeSSDPEventListener(OS::AutoRef<SSDPEventListener> listener);
	};
}

#endif
