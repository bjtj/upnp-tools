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
#include "UPnPTerms.hpp"

namespace UPNP {

	/**
	 * @brief 
	 */
	class UPnPDeviceSession : public UPnPCache {
	private:
		UDN _udn;
		bool _completed;
		OS::AutoRef<UPnPDevice> rootDevice;
	public:
		UPnPDeviceSession(const UDN & udn);
		virtual ~UPnPDeviceSession();
		UDN & udn();
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
		std::map< UDN, OS::AutoRef<UPnPDeviceSession> > sessions;
		OS::AutoRef<OnSessionOutdatedListener> onSessionOutdatedListener;
		
	public:
		UPnPDeviceSessionManager();
		virtual ~UPnPDeviceSessionManager();
		bool contains(const UDN & udn);
		void clear();
		OS::AutoRef<UPnPDeviceSession> prepareSession(const UDN & udn);
		void remove(const UDN & udn);
		size_t size();
		OS::AutoRef<UPnPDevice> findDevice(const UDN & udn);
		std::vector<OS::AutoRef<UPnPDeviceSession> > getSessions();
		std::vector<OS::AutoRef<UPnPDevice> > getDevices();
		OS::AutoRef<UPnPDeviceSession> operator[] (const UDN & udn);
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
		void addDevice(const SSDP::SSDPHeader & header);
		void removeDevice(const SSDP::SSDPHeader & header);
		OS::AutoRef<UPnPDevice> buildDevice(const SSDP::SSDPHeader & header);
		void onDeviceBuildCompleted(OS::AutoRef<UPnPDeviceSession> session);
		void onDeviceBuildFailed(OS::AutoRef<UPnPDeviceSession> session);
		OS::AutoRef<UPnPDevice> findDevice(const UDN & udn);
		void clearDevices();
		void sendMsearchAndWait(const std::string & target, unsigned long timeoutSec);
		void sendMsearchAsync(const std::string & target, unsigned long timeoutSec);
		UPnPDeviceSessionManager & sessionManager();
		std::vector<OS::AutoRef<UPnPDevice> > getDevices();
		HTTP::Url getBaseUrlByUdn(const UDN & udn);
		OS::AutoRef<UPnPService> getServiceByUdnAndServiceType(const UDN & udn, const std::string & serviceType);
		UPnPActionInvoker prepareActionInvoke(const UDN & udn, const std::string & serviceType);
		void subscribe(const UDN & udn, const std::string & serviceType);
		void unsubscribe(const UDN & udn, const std::string & serviceType);
		UPnPEventSubscriber prepareEventSubscriber(const UDN & udn, const std::string & serviceType);
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
