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

	void AnotherSSDPServer::sendMsearch(const std::string & st, unsigned long timeoutSec) {
		SSDPMsearchSender sender;
		sender.setSSDPPacketHandler(handler);
		sender.sendMsearchAllInterfaces(st, timeoutSec, "239.255.255.250", 1900);
		sender.gather(timeoutSec * 1000);
		sender.close();
	}
	void AnotherSSDPServer::setSSDPPacketHandler(SSDPPacketHandler * handler) {
		this->handler = handler;
		mcastListener.setSSDPPacketHandler(handler);
	}

}