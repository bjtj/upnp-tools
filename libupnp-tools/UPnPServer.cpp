#include "UPnPServer.hpp"

namespace UPNP {

	UPnPServer::UPnPServer() :
		ssdpServer(NULL), httpServer(NULL), addRemoveListener(NULL) {
	}
	
	UPnPServer::~UPnPServer() {
	}
	
	void UPnPServer::start() {
		if (ssdpServer == NULL) {
			ssdpServer = new SSDPServer;
			ssdpServer->startAsync();
		}
	}
	
	void UPnPServer::stop() {
		if (ssdpServer) {
			ssdpServer->stop();
			delete ssdpServer;
		}
	}
	
	bool UPnPServer::isRunning() {
		return ssdpServer != NULL;
	}
	
	void UPnPServer::setOnDeviceAddRemoveListener(OnDeviceAddRemoveListener * addRemoveListener) {
		this->addRemoveListener = addRemoveListener;
	}
	
}
