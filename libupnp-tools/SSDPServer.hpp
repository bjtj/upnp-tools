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

	/**
	 * @brief 
	 */
	class SSDPServer {
	private:
		UTIL::AutoRef<OS::Selector> selector;
		SSDP::SSDPMulticastListener mcastListener;
		UTIL::AutoRef<SSDP::SSDPEventHandler> handler;
		OS::Thread * pollingThread;
		OS::Thread * msearchResponseListenerThread;

	private:
		// copy not allowed
		SSDPServer(const SSDPServer & other);
		SSDPServer & operator= (const SSDPServer & other);

	public:
		SSDPServer();
		virtual ~SSDPServer();
		void start();
		void startAsync();
		void stop();
        void poll(unsigned long timeout);
		bool isPollingThreadRunning();
		void startPollingThread();
		void stopPollingThread();
		bool isMSearchResponseListenerThreadRunning();
		void startMSearchResponseListenerThread();
		void stopMSearchResponseListenerThread();
		void supportAsync(bool support);
		void sendMsearchAsync(const std::string & st, unsigned long timeoutSec);
		void sendMsearchAsync(const std::vector<std::string> & st, unsigned long timeoutSec);
		void sendMsearchAndGather(const std::string & st, unsigned long timeoutSec);
		void sendMsearchAndGather(std::vector<std::string> & st, unsigned long timeoutSec);
		UTIL::AutoRef<SSDPMsearchSender> sendMsearch(const std::string & st, unsigned long timeoutSec);
		UTIL::AutoRef<SSDPMsearchSender> sendMsearch(const std::string & st, unsigned long timeoutSec, UTIL::AutoRef<OS::Selector> selector);
		UTIL::AutoRef<SSDPMsearchSender> sendMsearch(const std::vector<std::string> & st, unsigned long timeoutSec);
		UTIL::AutoRef<SSDPMsearchSender> sendMsearch(const std::vector<std::string> & st, unsigned long timeoutSec, UTIL::AutoRef<OS::Selector> selector);
		void addSSDPEventHandler(UTIL::AutoRef<SSDPEventHandler> handler);
	};
}

#endif
