#ifndef __UPNP_SERVER_HPP__
#define __UPNP_SERVER_HPP__

#include <string>
#include <vector>

#include "UPnPDevice.hpp"

namespace UPNP {

	class OnDeviceAddListener {
	private:
	public:
		OnDeviceAddListener(){}
		virtual ~OnDeviceAddListener(){}

		virtual void onDeviceAdd(UPnPDevice & device) = 0;
	};


	class OnDeviceRemoveListener {
	private:
	public:
		OnDeviceRemoveListener(){}
		virtual ~OnDeviceRemoveListener(){}

		virtual void onDeviceRemove(UPnPDevice & device) = 0;
	};

	
	
	class UPnPServer {
	private:
		
		std::vector<UPnPDevice*> devices;
		
		SSDP::SSDPServer * ssdpServer;
		HTTP::HttpServer * httpServer;

		OnDeviceAddListener * addListener;
		OnDeviceRemoveListener * removeListener;
		
	public:
		
		UPnPServer();
		virtual ~UPnPServer();

		virtual void start();
		virtual void stop();
		virtual bool isRunning();

		void setOnDeviceAddListener(OnDeviceAddListener * addListener);
		void setOnDeviceRemoveListener(OnDeviceRemoveListener * removeListener);
	};
	
}

#endif
