#ifndef __ANOTHER_SSDP_SERVER_HPP__
#define __ANOTHER_SSDP_SERVER_HPP__

#include "SSDPHeader.hpp"
#include "SSDPMsearchSender.hpp"
#include "SSDPMulticastListener.hpp"

namespace SSDP {

	class AnotherSSDPServer {
	private:
		SSDP::SSDPMulticastListener mcastListener;
		SSDP::SSDPPacketHandler * handler;

	public:
		AnotherSSDPServer();
		virtual ~AnotherSSDPServer();

		void start();
		void stop();

		void sendMsearch(const std::string & st, unsigned long timeoutSec);
		void setSSDPPacketHandler(SSDPPacketHandler * handler);
	};
}

#endif
