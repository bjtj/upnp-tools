#include "UPnPControlPoint.hpp"
#include "UPnPDeviceDeserializer.hpp"
#include "UPnPDeviceBuilder.hpp"
#include "NetworkUtil.hpp"
#include "UPnPTerms.hpp"
#include "UPnPExceptions.hpp"
#include <liboslayer/Uuid.hpp>
#include <liboslayer/Logger.hpp>
#include <liboslayer/File.hpp>


namespace UPNP {

	using namespace UTIL;
	using namespace HTTP;
	using namespace SSDP;
	using namespace std;
	using namespace OS;
	

	static AutoRef<Logger> logger = LoggerFactory::instance().
		getObservingLogger(File::basename(__FILE__));
	

	UPnPDeviceSession::UPnPDeviceSession(const UDN & udn)
		: _udn(udn), _completed(false) {
	}
	
	UPnPDeviceSession::~UPnPDeviceSession() {
	}

	UDN & UPnPDeviceSession::udn() {
		return _udn;
	}

	bool UPnPDeviceSession::isCompleted() {
		return _completed;
	}

	void UPnPDeviceSession::setCompleted(bool completed) {
		this->_completed = completed;
	}

	AutoRef<UPnPDevice> UPnPDeviceSession::getRootDevice() {
		return rootDevice;
	}

	void UPnPDeviceSession::setRootDevice(AutoRef<UPnPDevice> device) {
		rootDevice = device;
	}
	

	/**
	 * @brief
	 */
	class UPnPControlPointLifetimeTask : public TimerTask {
	private:
		UPnPControlPoint & cp;
	public:
		UPnPControlPointLifetimeTask(UPnPControlPoint & cp) : cp(cp) {}
		virtual ~UPnPControlPointLifetimeTask() {}
		virtual void onTask() {
			cp.collectExpired();
		}
	};

	UPnPDeviceSessionManager::UPnPDeviceSessionManager() {
	}

	UPnPDeviceSessionManager::~UPnPDeviceSessionManager() {
	}

	bool UPnPDeviceSessionManager::contains(const UDN & udn) {
		return (sessions.find(udn) != sessions.end());
	}

	void UPnPDeviceSessionManager::clear() {
		sessions.clear();
	}

	AutoRef<UPnPDeviceSession> UPnPDeviceSessionManager::prepareSession(const UDN & udn) {
		if (!contains(udn)) {
			sessions[udn] = AutoRef<UPnPDeviceSession>(new UPnPDeviceSession(udn));
		}
		return sessions[udn];
	}

	void UPnPDeviceSessionManager::remove(const UDN & udn) {
		sessions.erase(udn);
	}

	size_t UPnPDeviceSessionManager::size() {
		return sessions.size();
	}

	AutoRef<UPnPDevice> UPnPDeviceSessionManager::findDevice(const UDN & udn) {
		AutoRef<UPnPDeviceSession> session = sessions[udn];
		if (!session.nil()) {
			return session->getRootDevice();
		}
		return AutoRef<UPnPDevice>();
	}

	vector< AutoRef<UPnPDeviceSession> > UPnPDeviceSessionManager::getSessions() {
		vector< AutoRef<UPnPDeviceSession> > ret;
		for (map< UDN, AutoRef<UPnPDeviceSession> >::iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
			ret.push_back(iter->second);
		}
		return ret;
	}

	vector< AutoRef<UPnPDevice> > UPnPDeviceSessionManager::getDevices() {
		vector< AutoRef<UPnPDevice> > ret;
		for (map< UDN, AutoRef<UPnPDeviceSession> >::iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
			ret.push_back(iter->second->getRootDevice());
		}
		return ret;
	}

	AutoRef<UPnPDeviceSession> UPnPDeviceSessionManager::operator[] (const UDN & udn) {
		return sessions[udn];
	}

	void UPnPDeviceSessionManager::collectExpired() {
		for (map< UDN, AutoRef<UPnPDeviceSession> >::iterator iter = sessions.begin(); iter != sessions.end();) {
			if (iter->second->expired()) {
				if (!onSessionOutdatedListener.nil()) {
					onSessionOutdatedListener->onSessionOutdated(iter->second);
				}
				sessions.erase(iter++);
			} else {
				iter++;
			}
		}
	}
	
	void UPnPDeviceSessionManager::setOnSessionOutdatedListener(AutoRef<UPnPDeviceSessionManager::OnSessionOutdatedListener> onSessionOutdatedListener) {
		this->onSessionOutdatedListener = onSessionOutdatedListener;
	}

	
	AutoRef<UPnPDeviceSessionManager::OnSessionOutdatedListener> UPnPDeviceSessionManager::getOnSessionOutdatedListener() {
		return onSessionOutdatedListener;
	}

	/**
	 * @brief
	 */
	class DeviceBuildTask : public Task {
	private:
		UPnPControlPoint & cp;
		AutoRef<UPnPDeviceSession> session;
		SSDPHeader header;
	public:
		DeviceBuildTask(UPnPControlPoint & cp, AutoRef<UPnPDeviceSession> session, const SSDPHeader & header)
			: cp(cp), session(session), header(header) {}
		virtual ~DeviceBuildTask() {}
		virtual void onTask() {
			try {
				session->setRootDevice(cp.buildDevice(header));
				session->setCompleted(true);
				cp.onDeviceBuildCompleted(session);
			} catch (Exception e) {
				logger->error("build device failed - " + e.message());
				cp.onDeviceBuildFailed(session);
			}
			session = NULL;
		}
	};

	/**
	 * @brief
	 */
	class ControlPointSSDPListener : public SSDPEventListener {
	private:
		UPnPControlPoint & cp;
	public:
		ControlPointSSDPListener(UPnPControlPoint & cp) : cp(cp) {
		}
		
		virtual ~ControlPointSSDPListener() {
		}
		
		virtual bool filter(const SSDPHeader & header) {
			UPnPDebug::instance().debug("ssdp", header.toString());
			return true;
		}
		
		virtual void onNotify(const SSDPHeader & header) {
			InetAddress addr = header.getRemoteAddr();
			if (header.isNotifyAlive()) {
				cp.addDevice(header);
			} else {
				cp.removeDevice(header);
			}
		}
		
		virtual void onMsearchResponse(const SSDPHeader & header) {
			InetAddress addr = header.getRemoteAddr();
			cp.addDevice(header);
		}
	};

	// 
	
	UPnPControlPoint::UPnPControlPoint(const UPnPControlPoint::Config & config)
		: config(config),
		  ssdpListener(new ControlPointSSDPListener(*this)),
		  started(false),
		  deviceBuildTaskThreadPool(10) {

		_init();
	}

	UPnPControlPoint::UPnPControlPoint(const UPnPControlPoint::Config & config, AutoRef<NetworkStateManager> networkStateManager)
		: networkStateManager(networkStateManager),
		  config(config),
		  ssdpListener(new ControlPointSSDPListener(*this)),
		  started(false),
		  deviceBuildTaskThreadPool(10) {

		_init();
	}
	
	UPnPControlPoint::~UPnPControlPoint() {
		/* destructor */
	}

	void UPnPControlPoint::_init() {

		/**
		 * session outdated listener
		 */
		class CPOnSessionOutdatedListener : public UPnPDeviceSessionManager::OnSessionOutdatedListener {
		private:
			UPnPControlPoint & cp;
		public:
			CPOnSessionOutdatedListener(UPnPControlPoint & cp) : cp(cp) {/**/}
			virtual ~CPOnSessionOutdatedListener() {/**/}
			virtual void onSessionOutdated(AutoRef<UPnPDeviceSession> session) {
				cp.onDeviceRemoved(session->getRootDevice());
			}
		};

		_sessionManager.setOnSessionOutdatedListener(AutoRef<UPnPDeviceSessionManager::OnSessionOutdatedListener>(new CPOnSessionOutdatedListener(*this)));
	}
		
	void UPnPControlPoint::startAsync() {

		if (started) {
			return;
		}
		
		ssdpServer.addSSDPEventListener(ssdpListener);
		ssdpServer.startAsync();
		ssdpServer.supportMsearchAsync(true);

		if (eventReceiver.nil()) {
			UPnPEventReceiverConfig eventReceiverConfig(config.getIntegerProperty("listen.port"));
			eventReceiver = AutoRef<UPnPEventReceiver>(new UPnPEventReceiver(eventReceiverConfig));
			eventReceiver->startAsync();
		}

		timerThread.start();
		timerThread.looper().interval(10 * 1000, AutoRef<TimerTask>(new UPnPControlPointLifetimeTask(*this)));

		deviceBuildTaskThreadPool.start();

		started = true;
	}

	void UPnPControlPoint::stop() {

		if (!started) {
			return;
		}

		deviceBuildTaskThreadPool.stop();

		timerThread.stop();
		timerThread.wait();
		
		if (eventReceiver.nil() == false) {
			eventReceiver->stop();
			eventReceiver = NULL;
		}
		
		ssdpServer.stop();

		started = false;
	}

	void UPnPControlPoint::setDeviceListener(AutoRef<UPnPDeviceListener> deviceListener) {
		this->deviceListener = deviceListener;
	}

	void UPnPControlPoint::addDevice(const SSDPHeader & header) {;
		UDN udn("uuid:" + UPNP::USN(header.getUsn()).uuid());
		InetAddress addr = header.getRemoteAddr();
		unsigned long timeout = parseCacheControlMilli(header.getCacheControl());
		if (_sessionManager.contains(udn)) {
			_sessionManager[udn]->updateTime();
			_sessionManager[udn]->timeout() = timeout;
		} else {
			AutoRef<UPnPDeviceSession> session = _sessionManager.prepareSession(udn);
			session->updateTime();
			session->timeout() = timeout;
			deviceBuildTaskThreadPool.setTask(AutoRef<Task>(new DeviceBuildTask(*this, session, header)));
		}
	}

	void UPnPControlPoint::removeDevice(const SSDPHeader & header) {
		UDN udn("uuid:" + UPNP::USN(header.getUsn()).uuid());
		if (!deviceListener.nil() && !_sessionManager[udn].nil()) {
			AutoRef<UPnPDevice> device = _sessionManager[udn]->getRootDevice();
			onDeviceRemoved(device);
		}
		_sessionManager.remove(udn);
	}

	AutoRef<UPnPDevice> UPnPControlPoint::buildDevice(const SSDPHeader & header) {
		UPnPDeviceBuilder builder(Url(header.getLocation()));
		builder.allow_fail_scpd() = true;
		return builder.execute();
	}

	void UPnPControlPoint::onDeviceBuildCompleted(AutoRef<UPnPDeviceSession> session) {
		onDeviceAdded(session->getRootDevice());
	}
	
	void UPnPControlPoint::onDeviceBuildFailed(AutoRef<UPnPDeviceSession> session) {
		_sessionManager.remove(session->udn());
	}

	AutoRef<UPnPDevice> UPnPControlPoint::findDevice(const UDN & udn) {
		return _sessionManager.findDevice(udn);
	}

	void UPnPControlPoint::clearDevices() {
		_sessionManager.clear();
	}
	
	void UPnPControlPoint::sendMsearchAndWait(const string & target, unsigned long timeoutSec) {
		ssdpServer.sendMsearchAndGather(target, timeoutSec);
	}

	void UPnPControlPoint::sendMsearchAsync(const string & target, unsigned long timeoutSec) {
		ssdpServer.sendMsearchAsync(target, timeoutSec);
	}

	UPnPDeviceSessionManager & UPnPControlPoint::sessionManager() {
		return _sessionManager;
	}
	
	vector< AutoRef<UPnPDevice> > UPnPControlPoint::getDevices() {
		return _sessionManager.getDevices();
	}

	Url UPnPControlPoint::getBaseUrlByUdn(const UDN & udn) {
		return findDevice(udn)->baseUrl();
	}

	AutoRef<UPnPService> UPnPControlPoint::getServiceByUdnAndServiceType(const UDN & udn, const string & serviceType) {
		AutoRef<UPnPDevice> device = findDevice(udn);
		return device->getService(serviceType);
	}

	UPnPActionInvoker UPnPControlPoint::prepareActionInvoke(const UDN & udn, const string & serviceType) {
		AutoRef<UPnPDevice> device = findDevice(udn);
		AutoRef<UPnPService> service = device->getService(serviceType);
		if (service.nil()) {
			throw Exception("service not found / type : " + serviceType);
		}
		return UPnPActionInvoker(device->baseUrl().relativePath(service->controlUrl()));
	}

	void UPnPControlPoint::subscribe(const UDN & udn, const string & serviceType) {
		if (eventReceiver.nil()) {
			throw Exception("event receiver is stopped");
		}
		InetAddress addr = NetworkUtil::selectDefaultAddress();
		UPnPEventSubscriber subscriber = prepareEventSubscriber(udn, serviceType);
		UPnPEventSubscribeRequest request(eventReceiver->getCallbackUrl(addr.getHost()), 300);
		UPnPEventSubscribeResponse response = subscriber.subscribe(request);
		UPnPEventSubscription subscription(response.sid());
		subscription.udn() = udn;
		subscription.serviceType() = serviceType;
		eventReceiver->addSubscription(subscription);
	}

	void UPnPControlPoint::unsubscribe(const UDN & udn, const string & serviceType) {
		if (eventReceiver.nil()) {
			throw Exception("event receiver is stopped");
		}

		UPnPEventSubscription subscription = eventReceiver->findSubscriptionByUdnAndServiceType(udn, serviceType);
		UPnPEventSubscriber subscriber = prepareEventSubscriber(udn, serviceType);
		subscriber.unsubscribe(subscription.sid());
	}

	UPnPEventSubscriber UPnPControlPoint::prepareEventSubscriber(const UDN & udn, const string & serviceType) {
		AutoRef<UPnPDevice> device = findDevice(udn);
		AutoRef<UPnPService> service = device->getService(serviceType);
		if (service.nil()) {
			throw Exception("service not found / type : " + serviceType);
		}
		return UPnPEventSubscriber(device->baseUrl().relativePath(service->eventSubUrl()));
	}

	AutoRef<UPnPEventReceiver> UPnPControlPoint::getEventReceiver() {
		return eventReceiver;
	}

	TimerLooperThread & UPnPControlPoint::getTimerThread() {
		return timerThread;
	}
	
	void UPnPControlPoint::collectExpired() {
		_sessionManager.collectExpired();
	}

	void UPnPControlPoint::addSharedDeviceList(AutoRef<SharedUPnPDeviceList> list) {
		sharedDeviceLists.push_back(list);
	}

	void UPnPControlPoint::removeSharedDeviceList(AutoRef<SharedUPnPDeviceList> list) {
		for (vector< AutoRef<SharedUPnPDeviceList> >::iterator iter = sharedDeviceLists.begin(); iter != sharedDeviceLists.end();) {
			if ((*iter) == list) {
				iter = sharedDeviceLists.erase(iter);
			} else {
				iter++;
			}
		}
	}

	void UPnPControlPoint::onDeviceAdded(AutoRef<UPnPDevice> device) {
		if (device.nil()) {
			return;
		}
		if (!deviceListener.nil()) {
			deviceListener->onDeviceAdded(device);
		}
		for (vector< AutoRef<SharedUPnPDeviceList> >::iterator iter = sharedDeviceLists.begin(); iter != sharedDeviceLists.end(); iter++) {
			(*iter)->add(device);
		}
	}
	
	void UPnPControlPoint::onDeviceRemoved(AutoRef<UPnPDevice> device) {
		if (device.nil()) {
			return;
		}
		if (!deviceListener.nil()) {
			deviceListener->onDeviceRemoved(device);
		}
		for (vector< AutoRef<SharedUPnPDeviceList> >::iterator iter = sharedDeviceLists.begin(); iter != sharedDeviceLists.end(); iter++) {
			(*iter)->remove(device);
		}
	}

	unsigned long UPnPControlPoint::parseCacheControlMilli(const string & cacheControl) {
		try {
			MaxAge maxAge = MaxAge::fromString(cacheControl);
			return maxAge.second() * 1000;
		} catch (UPnPParseException e) {
			logger->error("upnp parse exception - '" + e.toString() + "'");
			return DEFAULT_DEVICE_SESSION_TIMEOUT;
		}
	}

}
