#ifndef __SSDP_MSEARCH_SENDER_HPP__
#define __SSDP_MSEARCH_SENDER_HPP__

#include <liboslayer/os.hpp>
#include <liboslayer/DatagramSocket.hpp>
#include "SSDPPacketHandler.hpp"

namespace SSDP {

	class SSDPMsearchSender {
	private:
		OS::DatagramSocket sock;
		OS::Selector selector;
		SSDPPacketHandler * handler;
		bool _cancel;

	public:
		SSDPMsearchSender();
		SSDPMsearchSender(int port);
		SSDPMsearchSender(OS::InetAddress & bindAddr);
		virtual ~SSDPMsearchSender();
		void init();
		void close();
		void cancel();
	
		void gather(unsigned long timeout);
		void poll(unsigned long timeout);
		void onReceive(OS::DatagramPacket & packet);
		void sendMsearch(const std::string & st, unsigned long timeoutSec, const std::string & group, int port);
		void sendMcast(const std::string & content, const std::string & group, int port);
		void sendMsearchAllInterfaces(const std::string & st, unsigned long timeoutSec, const std::string & group, int port);
		void sendMcastToAllInterfaces(const std::string & content, const std::string & group, int port);
		std::string makeMsearchPacket(const std::string & st, unsigned long timeoutSec, const std::string & group, int port);
		void setSSDPPacketHandler(SSDPPacketHandler * handler);
	};
}

#endif