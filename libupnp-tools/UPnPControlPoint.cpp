#include "UPnPControlPoint.hpp"

namespace UPNP {

	using namespace std;

	/**
	 * @brief ssdp handler
	 */
	SSDPHandler::SSDPHandler() {
	}
	SSDPHandler::~SSDPHandler() {
	}
	void SSDPHandler::onNotify(HTTP::HttpHeader & header) {
		// request device description
	}

	/**
	 * @brief upnp control point
	 */
	UPnPControlPoint::UPnPControlPoint(int port, string searchTarget) : 
		searchTarget(searchTarget), httpServer(port), listener(NULL) {
	}

	UPnPControlPoint::~UPnPControlPoint() {
	}

	void UPnPControlPoint::startAsync() {

		ssdpServer.addNotifyHandler(&ssdpHandler);
		
		ssdpServer.startAsync();
		httpServer.startAsync();
	}

	void UPnPControlPoint::stop() {
		ssdpServer.stop();
		httpServer.stop();
	}

	bool UPnPControlPoint::isRunning() {
		return false;
	}

	void UPnPControlPoint::sendMsearch() {
		ssdpServer.sendMsearch(searchTarget);
	}

	vector<UPnPDevice> UPnPControlPoint::getDeviceList() {
		return devices;
	}

	UPnPDevice UPnPControlPoint::getDevice(std::string udn) {
		
		for (vector<UPnPDevice>::iterator iter = devices.begin(); 
			 iter != devices.end();
			 iter++) {
			
			UPnPDevice & device = *iter;
			if (!device.getUdn().compare(udn)) {
				return device;
			}
		}
		return UPnPDevice();
	}
	
	void UPnPControlPoint::removeAllDevices() {
		devices.clear();
	}
	
	void UPnPControlPoint::setSearchTarget(std::string searchTarget) {
		this->searchTarget = searchTarget;
	}
	
	std::string UPnPControlPoint::getSearchTaget() {
		return searchTarget;
	}
	
	void UPnPControlPoint::setOnDeviceAddRemoveListener(OnDeviceAddRemoveListener * listener) {
		this->listener = listener;
	}
}
