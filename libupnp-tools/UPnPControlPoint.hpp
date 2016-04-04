#ifndef __UPNP_CONTROL_POINT_HPP__
#define __UPNP_CONTROL_POINT_HPP__

#include <liboslayer/AutoRef.hpp>
#include <liboslayer/Timer.hpp>
#include <liboslayer/TaskThreadPool.hpp>
#include "SSDPServer.hpp"
#include "UPnPModels.hpp"
#include "UPnPSessionManager.hpp"
#include "UPnPActionInvoker.hpp"
#include "UPnPEventSubscriber.hpp"
#include "UPnPNotificationServer.hpp"

namespace UPNP {

	/**
	 * @brief
	 */
	class UPnPControlPointConfig : public UTIL::Properties {
	private:
	public:
		UPnPControlPointConfig() {}
		UPnPControlPointConfig(int port) {setProperty("listen.port", port);}
		virtual ~UPnPControlPointConfig() {}
	};


	/**
	 * @brief device add remove listener
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
	 * @brief upnp control point
	 */
	class UPnPControlPoint {
	private:
		UPnPControlPointConfig config;
		UTIL::AutoRef<DeviceAddRemoveListener> deviceListener;
		UTIL::AutoRef<SSDP::SSDPEventHandler> ssdpHandler;
		SSDP::SSDPServer ssdpServer;
		UPnPSessionManager _sessionManager;
		UPnPNotificationServer * notificationServer;
		UTIL::TimerLooperThread timerThread;
		bool started;
		UTIL::TaskThreadPool deviceBuildTaskThreadPool;
		
	public:
		UPnPControlPoint(UPnPControlPointConfig & config);
		virtual ~UPnPControlPoint();
		void startAsync();
		void stop();
		void setDeviceAddRemoveListener(UTIL::AutoRef<DeviceAddRemoveListener> deviceListener);
		void addDevice(SSDP::SSDPHeader & header);
		void removeDevice(SSDP::SSDPHeader & header);
		void onDeviceBuildCompleted(UTIL::AutoRef<UPnPSession> session);
		void onDeviceBuildFailed(UTIL::AutoRef<UPnPSession> session);
		UTIL::AutoRef<UPnPDevice> getDevice(const std::string & udn);
		void clearDevices();
		void sendMsearchAndWait(const std::string & target, unsigned long timeoutSec);
		UPnPSessionManager & sessionManager();
		HTTP::Url getBaseUrlWithUdn(const std::string & udn);
		UTIL::AutoRef<UPnPService> getServiceWithUdnAndServiceType(const std::string & udn, const std::string & serviceType);
		UPnPActionInvoker prepareActionInvoke(const std::string & udn, const std::string & serviceType);
		void subscribe(const std::string & udn, const std::string & serviceType);
		void unsubscribe(const std::string & udn, const std::string & serviceType);
		UPnPEventSubscriber prepareEventSubscriber(const std::string & udn, const std::string & serviceType);
		UPnPNotificationServer * getNotificationServer();
		UTIL::TimerLooperThread & getTimerThread();
		void clearOudatedSessions();
		unsigned long parseCacheControlMilli(const std::string & cacheControl, unsigned long def);
	};
}

#endif
