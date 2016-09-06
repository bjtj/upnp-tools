#ifndef __UPNP_CONTROL_POINT_HPP__
#define __UPNP_CONTROL_POINT_HPP__

#include <liboslayer/AutoRef.hpp>
#include <liboslayer/Condition.hpp>
#include <liboslayer/Timer.hpp>
#include <liboslayer/TaskThreadPool.hpp>
#include "SSDPServer.hpp"
#include "UPnPModels.hpp"
#include "UPnPActionInvoker.hpp"
#include "UPnPEventSubscriber.hpp"
#include "UPnPEventReceiver.hpp"
#include "NetworkStateManager.hpp"
#include "SharedUPnPDeviceList.hpp"
#include "UPnPCache.hpp"

namespace UPNP {

	/**
	 * @brief 
	 */
	class UPnPDeviceSession : public UPnPCache {
	private:
		std::string _udn;
		bool _completed;
		UTIL::AutoRef<UPnPDevice> rootDevice;
	public:
		UPnPDeviceSession(const std::string & udn);
		virtual ~UPnPDeviceSession();
		std::string & udn();
		bool isCompleted();
		void setCompleted(bool completed);
		UTIL::AutoRef<UPnPDevice> getRootDevice();
		void setRootDevice(UTIL::AutoRef<UPnPDevice> device);
	};


	/**
	 * @brief 
	 */
	class UPnPDeviceSessionManager {
	public:

		/**
		 * @brief 
		 */
		class OnSessionOutdatedListener {
		public:
			OnSessionOutdatedListener() {}
			virtual ~OnSessionOutdatedListener() {}
			virtual void onSessionOutdated(UTIL::AutoRef<UPnPDeviceSession> session) = 0;
		};

	private:
		std::map<std::string, UTIL::AutoRef<UPnPDeviceSession> > sessions;
		UTIL::AutoRef<OnSessionOutdatedListener> onSessionOutdatedListener;
		
	public:
		UPnPDeviceSessionManager();
		virtual ~UPnPDeviceSessionManager();
		bool has(const std::string & udn);
		void clear();
		UTIL::AutoRef<UPnPDeviceSession> prepareSession(const std::string & udn);
		void remove(const std::string & udn);
		size_t size();
		std::vector<std::string> getUdnS();
		UTIL::AutoRef<UPnPDevice> getDevice(const std::string & udn);
		std::vector<UTIL::AutoRef<UPnPDeviceSession> > getSessions();
		std::vector<UTIL::AutoRef<UPnPDevice> > getDevices();
		UTIL::AutoRef<UPnPDeviceSession> operator[] (const std::string & udn);
		void collectOutdated();
		void setOnSessionOutdatedListener(UTIL::AutoRef<OnSessionOutdatedListener> onSessionOutdatedListener);
		UTIL::AutoRef<OnSessionOutdatedListener> getOnSessionOutdatedListener();
	};
	

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

		static const unsigned long DEFAULT_DEVICE_SESSION_TIMEOUT = 1800 * 1000; 
		
		UTIL::AutoRef<NetworkStateManager> networkStateManager;
		UPnPControlPointConfig config;
		UTIL::AutoRef<DeviceAddRemoveListener> deviceListener;
		UTIL::AutoRef<SSDP::SSDPEventListener> ssdpListener;
		SSDP::SSDPServer ssdpServer;
		UPnPDeviceSessionManager _sessionManager;
		UPnPEventReceiver * eventReceiver;
		UTIL::TimerLooperThread timerThread;
		bool started;
		UTIL::TaskThreadPool deviceBuildTaskThreadPool;
		std::vector<UTIL::AutoRef<SharedUPnPDeviceList> > sharedDeviceLists;

	private:
		// do not allow copy or assign
		UPnPControlPoint(const UPnPControlPoint & other);
		UPnPControlPoint & operator=(const UPnPControlPoint & other);

	private:
		void init();
		
	public:
		UPnPControlPoint(UPnPControlPointConfig & config);
		UPnPControlPoint(UPnPControlPointConfig & config, UTIL::AutoRef<NetworkStateManager> networkStateManager);
		virtual ~UPnPControlPoint();
		void startAsync();
		void stop();
		void setDeviceAddRemoveListener(UTIL::AutoRef<DeviceAddRemoveListener> deviceListener);
		void addDevice(SSDP::SSDPHeader & header);
		void removeDevice(SSDP::SSDPHeader & header);
		UTIL::AutoRef<UPnPDevice> buildDevice(SSDP::SSDPHeader & header);
		void onDeviceBuildCompleted(UTIL::AutoRef<UPnPDeviceSession> session);
		void onDeviceBuildFailed(UTIL::AutoRef<UPnPDeviceSession> session);
		UTIL::AutoRef<UPnPDevice> getDevice(const std::string & udn);
		void clearDevices();
		void sendMsearchAndWait(const std::string & target, unsigned long timeoutSec);
		void sendMsearchAsync(const std::string & target, unsigned long timeoutSec);
		UPnPDeviceSessionManager & sessionManager();
		std::vector<UTIL::AutoRef<UPnPDevice> > getDevices();
		HTTP::Url getBaseUrlByUdn(const std::string & udn);
		UTIL::AutoRef<UPnPService> getServiceByUdnAndServiceType(const std::string & udn, const std::string & serviceType);
		UPnPActionInvoker prepareActionInvoke(const std::string & udn, const std::string & serviceType);
		void subscribe(const std::string & udn, const std::string & serviceType);
		void unsubscribe(const std::string & udn, const std::string & serviceType);
		UPnPEventSubscriber prepareEventSubscriber(const std::string & udn, const std::string & serviceType);
		UTIL::AutoRef<UPnPService> findService(UTIL::AutoRef<UPnPDevice> device, const std::string & serviceType);
		UTIL::AutoRef<UPnPService> findServiceRecursive(UTIL::AutoRef<UPnPDevice> device, const std::string & serviceType);
		UPnPEventReceiver * getEventReceiver();
		UTIL::TimerLooperThread & getTimerThread();
		void collectOutdated();
		unsigned long parseCacheControlMilli(const std::string & cacheControl, unsigned long def);
		void addSharedDeviceList(UTIL::AutoRef<SharedUPnPDeviceList> list);
		void removeSharedDeviceList(UTIL::AutoRef<SharedUPnPDeviceList> list);
		void announceDeviceAdded(UTIL::AutoRef<UPnPDevice> device);
		void announceDeviceRemoved(UTIL::AutoRef<UPnPDevice> device);
		unsigned long parseCacheControlMilli(const std::string & cacheControl);
	};
}

#endif
