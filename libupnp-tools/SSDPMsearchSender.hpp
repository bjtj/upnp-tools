#ifndef __SSDP_MSEARCH_SENDER_HPP__
#define __SSDP_MSEARCH_SENDER_HPP__

#include <liboslayer/os.hpp>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/DatagramSocket.hpp>
#include "SSDPEventListener.hpp"
#include "SSDPMulticastSender.hpp"

namespace ssdp {

    class SSDPMsearchSender : public SSDPMulticastSender {
    private:
	bool _cancel;
	unsigned long _timeout_milli;
    public:
	SSDPMsearchSender();
	SSDPMsearchSender(osl::AutoRef<osl::Selector> selector);
	SSDPMsearchSender(int port);
	SSDPMsearchSender(int port, osl::AutoRef<osl::Selector> selector);
	SSDPMsearchSender(osl::InetAddress & bindAddr);
	SSDPMsearchSender(osl::InetAddress & bindAddr, osl::AutoRef<osl::Selector> selector);
	virtual ~SSDPMsearchSender();
	void cancel();
	void gather();
	void gather(unsigned long timeout);
	void sendMsearch(const std::string & st, unsigned long timeoutSec);
	void sendMsearch(const std::string & st, unsigned long timeoutSec, const std::string & group, int port);
	void sendMsearchAllInterfaces(const std::string & st, unsigned long timeoutSec, const std::string & group, int port);
	std::string makeMsearchPacket(const std::string & st, unsigned long timeoutSec, const std::string & group, int port);
		
	void unicast(const std::string & content, osl::InetAddress & remoteAddr);
    };
}

#endif
