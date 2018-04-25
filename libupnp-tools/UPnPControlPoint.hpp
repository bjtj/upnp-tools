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

namespace upnp {

	/**
	 * @brief 
	 */
	class UPnPDeviceSession : public UPnPCache {
	private:
		UDN _udn;
		bool _completed;
		osl::AutoRef<UPnPDevice> rootDevice;
	public:
		UPnPDeviceSession(const UDN & udn);
		virtual ~UPnPDeviceSession();
		UDN & udn();
		bool isCompleted();
		void setCompleted(bool completed);
		osl::AutoRef<UPnPDevice> getRootDevice();
		void setRootDevice(osl::AutoRef<UPnPDevice> device);
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
			virtual void onSessionOutdated(osl::AutoRef<UPnPDeviceSession> session) = 0;
		};

	private:
		std::map< UDN, osl::AutoRef<UPnPDeviceSession> > sessions;
		osl::AutoRef<OnSessionOutdatedListener> onSessionOutdatedListener;
		
	public:
		UPnPDeviceSessionManager();
		virtual ~UPnPDeviceSessionManager();
		bool contains(const UDN & udn);
		void clear();
		osl::AutoRef<UPnPDeviceSession> prepareSession(const UDN & udn);
		void remove(const UDN & udn);
		size_t size();
		osl::AutoRef<UPnPDevice> findDevice(const UDN & udn);
		std::vector<osl::AutoRef<UPnPDeviceSession> > getSessions();
		std::vector<osl::AutoRef<UPnPDevice> > getDevices();
		osl::AutoRef<UPnPDeviceSession> operator[] (const UDN & udn);
		void collectExpired();
		void setOnSessionOutdatedListener(osl::AutoRef<OnSessionOutdatedListener> onSessionOutdatedListener);
		osl::AutoRef<OnSessionOutdatedListener> getOnSessionOutdatedListener();
	};

	/**
	 * @brief upnp control point
	 */
	class UPnPControlPoint {
	public:
		class Config : public osl::Properties {
		public:
			Config() {/**/}
			Config(int port) {
				setProperty("listen.port", port);
			}
			virtual ~Config() {/**/}
		};
		
	private:
		static const unsigned long DEFAULT_DEVICE_SESSION_TIMEOUT = 1800 * 1000; 
		osl::AutoRef<NetworkStateManager> networkStateManager;
		Config config;
		osl::AutoRef<UPnPDeviceListener> deviceListener;
		osl::AutoRef<ssdp::SSDPEventListener> ssdpListener;
		ssdp::SSDPServer ssdpServer;
		UPnPDeviceSessionManager _sessionManager;
		osl::AutoRef<UPnPEventReceiver> eventReceiver;
		osl::TimerLooperThread timerThread;
		bool started;
		osl::TaskThreadPool deviceBuildTaskThreadPool;
		std::vector<osl::AutoRef<SharedUPnPDeviceList> > sharedDeviceLists;

	private:
		// do not allow copy or assign
		UPnPControlPoint(const UPnPControlPoint & other);
		UPnPControlPoint & operator=(const UPnPControlPoint & other);
		
	public:
		UPnPControlPoint(const Config & config);
		UPnPControlPoint(const Config & config, osl::AutoRef<NetworkStateManager> networkStateManager);
		virtual ~UPnPControlPoint();
		
	private:
		void _init();
		
	public:
		void startAsync();
		void stop();
		void setDeviceListener(osl::AutoRef<UPnPDeviceListener> deviceListener);
		void addDevice(const ssdp::SSDPHeader & header);
		void removeDevice(const ssdp::SSDPHeader & header);
		osl::AutoRef<UPnPDevice> buildDevice(const ssdp::SSDPHeader & header);
		void onDeviceBuildCompleted(osl::AutoRef<UPnPDeviceSession> session);
		void onDeviceBuildFailed(osl::AutoRef<UPnPDeviceSession> session);
		osl::AutoRef<UPnPDevice> findDevice(const UDN & udn);
		void clearDevices();
		void sendMsearchAndWait(const std::string & target, unsigned long timeoutSec);
		void sendMsearchAsync(const std::string & target, unsigned long timeoutSec);
		UPnPDeviceSessionManager & sessionManager();
		std::vector<osl::AutoRef<UPnPDevice> > getDevices();
		http::Url getBaseUrlByUdn(const UDN & udn);
		osl::AutoRef<UPnPService> getServiceByUdnAndServiceType(const UDN & udn, const std::string & serviceType);
		UPnPActionInvoker prepareActionInvoke(const UDN & udn, const std::string & serviceType);
		void subscribe(const UDN & udn, const std::string & serviceType);
		void unsubscribe(const UDN & udn, const std::string & serviceType);
		UPnPEventSubscriber prepareEventSubscriber(const UDN & udn, const std::string & serviceType);
		osl::AutoRef<UPnPEventReceiver> getEventReceiver();
		osl::TimerLooperThread & getTimerThread();
		void collectExpired();
		unsigned long parseCacheControlMilli(const std::string & cacheControl, unsigned long def);
		void addSharedDeviceList(osl::AutoRef<SharedUPnPDeviceList> list);
		void removeSharedDeviceList(osl::AutoRef<SharedUPnPDeviceList> list);
		void onDeviceAdded(osl::AutoRef<UPnPDevice> device);
		void onDeviceRemoved(osl::AutoRef<UPnPDevice> device);
		unsigned long parseCacheControlMilli(const std::string & cacheControl);
	};
}

#endif
