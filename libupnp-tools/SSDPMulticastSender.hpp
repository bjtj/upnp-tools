#ifndef __SSDP_MULTICAST_SENDER_HPP__
#define __SSDP_MULTICAST_SENDER_HPP__

#include <vector>
#include <string>
#include "SSDPEventListener.hpp"
#include <liboslayer/os.hpp>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/DatagramSocket.hpp>

namespace ssdp {
	
	class SSDPMulticastSender {
	private:
		osl::DatagramSocket sock;
		osl::AutoRef<osl::Selector> selector;
		std::vector<osl::AutoRef<SSDPEventListener> > listeners;
		
	public:
		SSDPMulticastSender();
		SSDPMulticastSender(osl::AutoRef<osl::Selector> selector);
		SSDPMulticastSender(int port);
		SSDPMulticastSender(int port, osl::AutoRef<osl::Selector> selector);
		SSDPMulticastSender(osl::InetAddress & bindAddr);
		SSDPMulticastSender(osl::InetAddress & bindAddr, osl::AutoRef<osl::Selector> selector);
		virtual ~SSDPMulticastSender();
		void init();
		void close();
		void poll(unsigned long timeout);
		bool isReadable();
		bool isReadable(osl::Selector & selector);
		void procRead();
		void onReceive(osl::DatagramPacket & packet);
		void sendMcast(const std::string & content, const std::string & group, int port);
		void sendMcastToAllInterfaces(const std::string & content, const std::string & group, int port);
		void addSSDPEventListener(osl::AutoRef<SSDPEventListener> listener);
		void removeSSDPEventListener(osl::AutoRef<SSDPEventListener> listener);
	};
}

#endif
