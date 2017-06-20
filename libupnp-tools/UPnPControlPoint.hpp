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
#include "UPnPDebug.hpp"
#include "UPnPDeviceListener.hpp"

namespace UPNP {

	/**
	 * @brief 
	 */
	class UPnPDeviceSession : public UPnPCache {
	private:
		std::string _udn;
		bool _completed;
		OS::AutoRef<UPnPDevice> rootDevice;
	public:
		UPnPDeviceSession(const std::string & udn);
		virtual ~UPnPDeviceSession();
		std::string & udn();
		bool isCompleted();
		void setCompleted(bool completed);
		OS::AutoRef<UPnPDevice> getRootDevice();
		void setRootDevice(OS::AutoRef<UPnPDevice> device);
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
			virtual void onSessionOutdated(OS::AutoRef<UPnPDeviceSession> session) = 0;
		};

	private:
		std::map<std::string, OS::AutoRef<UPnPDeviceSession> > sessions;
		OS::AutoRef<OnSessionOutdatedListener> onSessionOutdatedListener;
		
	public:
		UPnPDeviceSessionManager();
		virtual ~UPnPDeviceSessionManager();
		bool has(const std::string & udn);
		void clear();
		OS::AutoRef<UPnPDeviceSession> prepareSession(const std::string & udn);
		void remove(const std::string & udn);
		size_t size();
		std::vector<std::string> getUdnS();
		OS::AutoRef<UPnPDevice> getDevice(const std::string & udn);
		std::vector<OS::AutoRef<UPnPDeviceSession> > getSessions();
		std::vector<OS::AutoRef<UPnPDevice> > getDevices();
		OS::AutoRef<UPnPDeviceSession> operator[] (const std::string & udn);
		void collectExpired();
		void setOnSessionOutdatedListener(OS::AutoRef<OnSessionOutdatedListener> onSessionOutdatedListener);
		OS::AutoRef<OnSessionOutdatedListener> getOnSessionOutdatedListener();
	};

	/**
	 * @brief upnp control point
	 */
	class UPnPControlPoint : public UPnPDebuggable {
	public:
		class Config : public UTIL::Properties {
		public:
			Config() {/**/}
			Config(int port) {
				setProperty("listen.port", port);
			}
			virtual ~Config() {/**/}
		};
		
	private:
		static const unsigned long DEFAULT_DEVICE_SESSION_TIMEOUT = 1800 * 1000; 
		OS::AutoRef<NetworkStateManager> networkStateManager;
		Config config;
		OS::AutoRef<UPnPDeviceListener> deviceListener;
		OS::AutoRef<SSDP::SSDPEventListener> ssdpListener;
		SSDP::SSDPServer ssdpServer;
		UPnPDeviceSessionManager _sessionManager;
		OS::AutoRef<UPnPEventReceiver> eventReceiver;
		UTIL::TimerLooperThread timerThread;
		bool started;
		UTIL::TaskThreadPool deviceBuildTaskThreadPool;
		std::vector<OS::AutoRef<SharedUPnPDeviceList> > sharedDeviceLists;

	private:
		// do not allow copy or assign
		UPnPControlPoint(const UPnPControlPoint & other);
		UPnPControlPoint & operator=(const UPnPControlPoint & other);
		
	public:
		UPnPControlPoint(const Config & config);
		UPnPControlPoint(const Config & config, OS::AutoRef<NetworkStateManager> networkStateManager);
		virtual ~UPnPControlPoint();
		
	private:
		void _init();
		
	public:
		void startAsync();
		void stop();
		void setDeviceListener(OS::AutoRef<UPnPDeviceListener> deviceListener);
		void addDevice(SSDP::SSDPHeader & header);
		void removeDevice(SSDP::SSDPHeader & header);
		OS::AutoRef<UPnPDevice> buildDevice(SSDP::SSDPHeader & header);
		void onDeviceBuildCompleted(OS::AutoRef<UPnPDeviceSession> session);
		void onDeviceBuildFailed(OS::AutoRef<UPnPDeviceSession> session);
		OS::AutoRef<UPnPDevice> getDevice(const std::string & udn);
		void clearDevices();
		void sendMsearchAndWait(const std::string & target, unsigned long timeoutSec);
		void sendMsearchAsync(const std::string & target, unsigned long timeoutSec);
		UPnPDeviceSessionManager & sessionManager();
		std::vector<OS::AutoRef<UPnPDevice> > getDevices();
		HTTP::Url getBaseUrlByUdn(const std::string & udn);
		OS::AutoRef<UPnPService> getServiceByUdnAndServiceType(const std::string & udn, const std::string & serviceType);
		UPnPActionInvoker prepareActionInvoke(const std::string & udn, const std::string & serviceType);
		void subscribe(const std::string & udn, const std::string & serviceType);
		void unsubscribe(const std::string & udn, const std::string & serviceType);
		UPnPEventSubscriber prepareEventSubscriber(const std::string & udn, const std::string & serviceType);
		OS::AutoRef<UPnPEventReceiver> getEventReceiver();
		UTIL::TimerLooperThread & getTimerThread();
		void collectExpired();
		unsigned long parseCacheControlMilli(const std::string & cacheControl, unsigned long def);
		void addSharedDeviceList(OS::AutoRef<SharedUPnPDeviceList> list);
		void removeSharedDeviceList(OS::AutoRef<SharedUPnPDeviceList> list);
		void onDeviceAdded(OS::AutoRef<UPnPDevice> device);
		void onDeviceRemoved(OS::AutoRef<UPnPDevice> device);
		unsigned long parseCacheControlMilli(const std::string & cacheControl);
	};
}

#endif
