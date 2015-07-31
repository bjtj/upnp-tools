#ifndef __UPNP_CONTROL_POINT_HPP__
#define __UPNP_CONTROL_POINT_HPP__

#include <libhttp-server/HttpHeader.hpp>

#include <string>
#include <vector>

#include "SSDPServer.hpp"
#include "UPnPDevice.hpp"

namespace UPNP {

	/**
	 * @brief device add remove listener
	 */
	class OnDeviceAddRemoveListener {
	public:
		OnDeviceAddRemoveListener() {}
		virtual ~OnDeviceAddRemoveListener() {}

		virtual void onDeviceAdd(UPnPDevice & device) = 0;
		virtual void onDeviceRemove(UPnPDevice & device) = 0;
	};

	/**
	 * @brief ssdp handler
	 */
	class SSDPHandler : public SSDP::OnMsearchHandler, public SSDP::OnNotifyHandler {
	public:
		SSDPHandler();
		virtual ~SSDPHandler();
		virtual void onMsearch(HTTP::HttpHeader & header);
		virtual void onNotify(HTTP::HttpHeader & header);
	};

	/**
	 * @brief upnp control point
	 */
	class UPnPControlPoint {
	private:
		SSDPServer ssdpServer;
		std::vector<UPnPDevice> devices;
		std::string searchTarget;
		OnDeviceAddRemoveListener * listener;
		
	public:
		UPnPControlPoint();
		virtual ~UPnPControlPoint();

		virtual void start();
		virtual void stop();
		virtual bool isRunning();

		virtual void sendMsearch();
		virtual vector<UPnPDevice> getDeviceList();
		virtual UPnPDevice getDevice(std::string udn);
		virtual void removeAllDevices();

		void setSearchTarget(std::string searchTarget);
		std::string getSearchTaget();

		void setOnDeviceAddRemoveListener(OnDeviceAddRemoveListener * listener);
	};

}

#endif
