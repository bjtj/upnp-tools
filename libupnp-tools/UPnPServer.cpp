#include "UPnPServer.hpp"

namespace UPNP {

	using namespace SSDP;

	UPnPServer::UPnPServer() : actionListener(NULL) {
	}
	
	UPnPServer::~UPnPServer() {
	}
	
	void UPnPServer::start() {
        ssdpServer.start();
	}
    
    void UPnPServer::startAsync() {
        ssdpServer.startAsync();
    }
    
    void UPnPServer::poll(unsigned long timeout) {
        ssdpServer.poll(timeout);
    }
	
	void UPnPServer::stop() {
        ssdpServer.stop();
	}
	
	bool UPnPServer::isRunning() {
        return ssdpServer.isRunning();
	}
    
    void UPnPServer::registerDevice(const UPnPDevice & device) {
        announceDeviceRecursive(device);
    }
    void UPnPServer::unregisterDevice(const std::string & udn) {
        if (devices.hasDevice(udn)) {
            UPnPDevice & device = devices.getDevice(udn);
            byebyeDeviceRecursive(device);
        }
    }
    
    void UPnPServer::announceDevice(const UPnPDevice & device) {
        
    }
    void UPnPServer::announceDeviceRecursive(const UPnPDevice & device) {
        
    }
    void UPnPServer::announceService(const UPnPService & service) {
        
    }
    
    void UPnPServer::byebyeDevice(const UPnPDevice & device) {
        
    }
    void UPnPServer::byebyeDeviceRecursive(const UPnPDevice & device) {
        
    }
    void UPnPServer::byebyeService(const UPnPService & service) {
        
    }
	
    void UPnPServer::setInvokeActionListener(InvokeActionListener * actionListener) {
        this->actionListener = actionListener;
    }
	
}
