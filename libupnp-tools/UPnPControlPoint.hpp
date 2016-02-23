#ifndef __UPNP_CONTROL_POINT_HPP__
#define __UPNP_CONTROL_POINT_HPP__

#include <liboslayer/AutoRef.hpp>
#include "SSDPServer.hpp"
#include "UPnPModels.hpp"
#include "UPnPSession.hpp"
#include "UPnPActionInvoker.hpp"

namespace UPNP {

	/**
	 *
	 */
	class DeviceAddRemoveListener {
	private:
	public:
		DeviceAddRemoveListener() {}
		virtual ~DeviceAddRemoveListener() {}

		virtual void onDeviceAdd(UTIL::AutoRef<UPnPDevice> device) {}
		virtual void onDeviceRemove(UTIL::AutoRef<UPnPDevice> device) {}
	};


	/**
	 *
	 */
	class UPnPControlPoint {
	private:
		UTIL::AutoRef<DeviceAddRemoveListener> deviceListener;
		UTIL::AutoRef<SSDP::SSDPEventHandler> ssdpHandler;
		SSDP::SSDPServer ssdpServer;
		UPnPSessionManager _sessionManager;
		
	public:
		UPnPControlPoint();
		virtual ~UPnPControlPoint();
		
		void startAsync();
		void stop();

		void setDeviceAddRemoveListener(UTIL::AutoRef<DeviceAddRemoveListener> deviceListener);

		void addDevice(SSDP::SSDPHeader & header);
		void removeDevice(SSDP::SSDPHeader & header);
		UTIL::AutoRef<UPnPDevice> getDevice(const std::string & udn);

		void clearDevices();
		void sendMsearchAndWait(const std::string & target, unsigned long timeoutSec);
		UPnPSessionManager & sessionManager();
		UPnPActionInvoker prepareActionInvoke(const std::string & udn, const std::string & serviceType);
	};
}

#endif
