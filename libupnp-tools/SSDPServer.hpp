#ifndef __ANOTHER_SSDP_SERVER_HPP__
#define __ANOTHER_SSDP_SERVER_HPP__

#include <string>
#include <vector>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/os.hpp>
#include "SSDPHeader.hpp"
#include "SSDPMsearchSender.hpp"
#include "SSDPMulticastListener.hpp"
#include "SSDPEventHandler.hpp"

namespace SSDP {

	class SSDPServer {
	private:
		SSDP::SSDPMulticastListener mcastListener;
		UTIL::AutoRef<SSDP::SSDPEventHandler> handler;
		OS::Thread * thread;

	public:
		SSDPServer();
		virtual ~SSDPServer();

		void start();
		void startAsync(unsigned long timeout);
		void stop();
        void poll(unsigned long timeout);

		void sendMsearchAndGather(const std::string & st, unsigned long timeoutSec);
		void sendMsearchAndGather(std::vector<std::string> & st, unsigned long timeoutSec);
		UTIL::AutoRef<SSDPMsearchSender> sendMsearch(const std::string & st, unsigned long timeoutSec);
		UTIL::AutoRef<SSDPMsearchSender> sendMsearch(std::vector<std::string> & st, unsigned long timeoutSec);
		void setSSDPEventHandler(UTIL::AutoRef<SSDPEventHandler> handler);
	};
}

#endif
