#ifndef __ANOTHER_SSDP_SERVER_HPP__
#define __ANOTHER_SSDP_SERVER_HPP__

#include <string>
#include <vector>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/os.hpp>
#include <liboslayer/Thread.hpp>
#include "SSDPHeader.hpp"
#include "SSDPMsearchSender.hpp"
#include "SSDPMulticastListener.hpp"
#include "SSDPEventListener.hpp"
#include "UPnPUtils.hpp"

namespace SSDP {

	/**
	 * @brief 
	 */
	class SSDPServer {
	private:
		OS::AutoRef<OS::Selector> selector;
		SSDP::SSDPMulticastListener mcastListener;
		OS::AutoRef<SSDP::SSDPEventListener> listener;
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
		void supportMsearchAsync(bool support);
		void sendMsearchAsync(const std::string & st, unsigned long timeoutSec);
		void sendMsearchAsync(const std::vector<std::string> & st, unsigned long timeoutSec);
		void sendMsearchAndGather(const std::string & st, unsigned long timeoutSec);
		void sendMsearchAndGather(std::vector<std::string> & st, unsigned long timeoutSec);
		OS::AutoRef<SSDPMsearchSender> sendMsearch(const std::string & st, unsigned long timeoutSec);
		OS::AutoRef<SSDPMsearchSender> sendMsearch(const std::string & st, unsigned long timeoutSec, OS::AutoRef<OS::Selector> selector);
		OS::AutoRef<SSDPMsearchSender> sendMsearch(const std::vector<std::string> & st, unsigned long timeoutSec);
		OS::AutoRef<SSDPMsearchSender> sendMsearch(const std::vector<std::string> & st, unsigned long timeoutSec, OS::AutoRef<OS::Selector> selector);
		void sendNotify(const SSDPHeader & header);
		SSDPHeader getNotifyHeader(const std::string & nts, const UPNP::USN & usn);
		void addSSDPEventListener(OS::AutoRef<SSDPEventListener> listener);
	};
}

#endif
