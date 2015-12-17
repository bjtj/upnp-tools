#include "AnotherSSDPServer.hpp"
#include "SSDPMsearchSender.hpp"

namespace SSDP {

	AnotherSSDPServer::AnotherSSDPServer() : mcastListener("239.255.255.250", 1900), handler(NULL) {
	}
	AnotherSSDPServer::~AnotherSSDPServer() {
	}

	void AnotherSSDPServer::start() {
		mcastListener.start();
	}
	void AnotherSSDPServer::stop() {
		mcastListener.stop();
	}
    void AnotherSSDPServer::poll(unsigned long timeout) {
        mcastListener.poll(timeout);
    }
	void AnotherSSDPServer::sendMsearchAndGather(const std::string & st, unsigned long timeoutSec) {
		UTIL::AutoRef<SSDPMsearchSender> sender = sendMsearch(st, timeoutSec);
		sender->gather(timeoutSec * 1000);
		sender->close();
	}
	void AnotherSSDPServer::sendMsearchAndGather(std::vector<std::string> & st, unsigned long timeoutSec) {
		UTIL::AutoRef<SSDPMsearchSender> sender = sendMsearch(st, timeoutSec);
		sender->gather(timeoutSec * 1000);
		sender->close();
	}
	UTIL::AutoRef<SSDPMsearchSender> AnotherSSDPServer::sendMsearch(const std::string & st, unsigned long timeoutSec) {
		SSDPMsearchSender * sender = new SSDPMsearchSender;
		sender->setSSDPPacketHandler(handler);
		sender->sendMsearchAllInterfaces(st, timeoutSec, "239.255.255.250", 1900);
		return UTIL::AutoRef<SSDPMsearchSender>(sender);
	}
	UTIL::AutoRef<SSDPMsearchSender> AnotherSSDPServer::sendMsearch(std::vector<std::string> & st, unsigned long timeoutSec) {
		SSDPMsearchSender * sender = new SSDPMsearchSender;
		sender->setSSDPPacketHandler(handler);
		for (std::vector<std::string>::iterator iter = st.begin(); iter != st.end(); iter++) {
			sender->sendMsearchAllInterfaces(*iter, timeoutSec, "239.255.255.250", 1900);
		}
		return UTIL::AutoRef<SSDPMsearchSender>(sender);
	}
	void AnotherSSDPServer::setSSDPPacketHandler(SSDPPacketHandler * handler) {
		this->handler = handler;
		mcastListener.setSSDPPacketHandler(handler);
	}

}
