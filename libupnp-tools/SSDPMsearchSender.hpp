#ifndef __SSDP_MSEARCH_SENDER_HPP__
#define __SSDP_MSEARCH_SENDER_HPP__

#include <liboslayer/os.hpp>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/DatagramSocket.hpp>
#include "SSDPEventListener.hpp"
#include "SSDPMulticastSender.hpp"

namespace SSDP {

	class SSDPMsearchSender : public SSDPMulticastSender {
	private:
		bool _cancel;

	public:
		SSDPMsearchSender();
		SSDPMsearchSender(UTIL::AutoRef<OS::Selector> selector);
		SSDPMsearchSender(int port);
		SSDPMsearchSender(int port, UTIL::AutoRef<OS::Selector> selector);
		SSDPMsearchSender(OS::InetAddress & bindAddr);
		SSDPMsearchSender(OS::InetAddress & bindAddr, UTIL::AutoRef<OS::Selector> selector);
		virtual ~SSDPMsearchSender();
		void cancel();
		void gather(unsigned long timeout);
		void sendMsearch(const std::string & st, unsigned long timeoutSec, const std::string & group, int port);
		void sendMsearchAllInterfaces(const std::string & st, unsigned long timeoutSec, const std::string & group, int port);
		std::string makeMsearchPacket(const std::string & st, unsigned long timeoutSec, const std::string & group, int port);
		
		void unicast(const std::string & content, OS::InetAddress & remoteAddr);
	};
}

#endif
