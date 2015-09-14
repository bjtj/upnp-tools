#ifndef __UPNP_SERVER_HPP__
#define __UPNP_SERVER_HPP__

#include <string>
#include <vector>

#include "UPnPDevice.hpp"

namespace UPNP {

	/**
	 * @brief device add remove listener
	 */
	class OnDeviceAddRemoveListener {
	private:
	public:
		OnDeviceAddListener(){}
		virtual ~OnDeviceAddListener(){}

		virtual void onDeviceAdd(UPnPDevice & device) = 0;
		virtual void onDeviceRemove(UPnPDevice & device) = 0;
	};

	/**
	 * @brief UPNP Server
	 */
	class UPnPServer {
	private:
		std::vector<UPnPDevice*> devices;
		SSDP::SSDPServer * ssdpServer;
		HTTP::HttpServer * httpServer;
		OnDeviceAddRemoveListener * addRemoveListener;
		
	public:
		UPnPServer();
		virtual ~UPnPServer();

		virtual void start();
		virtual void stop();
		virtual bool isRunning();

		void setOnDeviceAddRemoveListener(OnDeviceAddRemoveListener * addRemoveListener);
	};
	
}

#endif
