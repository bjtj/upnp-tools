#include "UPnPControlPoint.hpp"
#include "UPnPDeviceDeserializer.hpp"
#include "NetworkUtil.hpp"
#include "UPnPUtils.hpp"
#include "UPnPExceptions.hpp"
#include <liboslayer/Uuid.hpp>
#include <liboslayer/Logger.hpp>

namespace UPNP {

	using namespace UTIL;
	using namespace HTTP;
	using namespace SSDP;
	using namespace std;
	using namespace OS;

	static AutoRef<Logger> logger = LoggerFactory::getInstance().getObservingLogger(__FILE__);

	UPnPDeviceSession::UPnPDeviceSession(const string & udn)
		: _udn(udn), _completed(false) {
	}
	
	UPnPDeviceSession::~UPnPDeviceSession() {
	}

	string & UPnPDeviceSession::udn() {
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
		virtual void doTask() {
			cp.collectOutdated();
		}
	};



	UPnPDeviceSessionManager::UPnPDeviceSessionManager() {
	}
	UPnPDeviceSessionManager::~UPnPDeviceSessionManager() {
	}

	bool UPnPDeviceSessionManager::has(const string & udn) {
		return (sessions.find(udn) != sessions.end());
	}
	void UPnPDeviceSessionManager::clear() {
		sessions.clear();
	}
	AutoRef<UPnPDeviceSession> UPnPDeviceSessionManager::prepareSession(const string & udn) {
		if (!has(udn)) {
			sessions[udn] = AutoRef<UPnPDeviceSession>(new UPnPDeviceSession(udn));
		}
		return sessions[udn];
	}
	void UPnPDeviceSessionManager::remove(const string & udn) {
		sessions.erase(udn);
	}
	size_t UPnPDeviceSessionManager::size() {
		return sessions.size();
	}
	vector<string> UPnPDeviceSessionManager::getUdnS() {
		vector<string> ret;
		for (map<string, AutoRef<UPnPDeviceSession> >::iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
			ret.push_back(iter->first);
		}
		return ret;
	}
	AutoRef<UPnPDevice> UPnPDeviceSessionManager::getDevice(const string & udn) {
		AutoRef<UPnPDeviceSession> session = sessions[udn];
		if (!session.nil()) {
			return session->getRootDevice();
		}
		return AutoRef<UPnPDevice>();
	}
	vector<AutoRef<UPnPDeviceSession> > UPnPDeviceSessionManager::getSessions() {
		vector<AutoRef<UPnPDeviceSession> > ret;
		for (map<string, AutoRef<UPnPDeviceSession> >::iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
			ret.push_back(iter->second);
		}
		return ret;
	}
	vector<AutoRef<UPnPDevice> > UPnPDeviceSessionManager::getDevices() {
		vector<AutoRef<UPnPDevice> > ret;
		for (map<string, AutoRef<UPnPDeviceSession> >::iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
			ret.push_back(iter->second->getRootDevice());
		}
		return ret;
	}
	AutoRef<UPnPDeviceSession> UPnPDeviceSessionManager::operator[] (const string & udn) {
		return sessions[udn];
	}

	void UPnPDeviceSessionManager::collectOutdated() {
		for (map<string, AutoRef<UPnPDeviceSession> >::iterator iter = sessions.begin(); iter != sessions.end();) {
			if (iter->second->outdated()) {
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
		DeviceBuildTask(UPnPControlPoint & cp, AutoRef<UPnPDeviceSession> session, SSDPHeader & header) : cp(cp), session(session), header(header) {}
		virtual ~DeviceBuildTask() {}

		virtual void doTask() {
			try {
				session->setRootDevice(cp.buildDevice(header));
				session->setCompleted(true);
				cp.onDeviceBuildCompleted(session);
			} catch (Exception & e) {
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
		ControlPointSSDPListener(UPnPControlPoint & cp) : cp(cp) {}
		virtual ~ControlPointSSDPListener() {}
	
		virtual bool filter(SSDPHeader & header) {
			cp.debug("ssdp", header.toString());
			return true;
		}
		
		virtual void onNotify(SSDPHeader & header) {
			InetAddress addr = header.getRemoteAddr();
			if (header.isNotifyAlive()) {
				cp.addDevice(header);
			} else {
				cp.removeDevice(header);
			}
		}
		
		virtual void onMsearchResponse(SSDPHeader & header) {
			InetAddress addr = header.getRemoteAddr();
			Uuid uuid(header.getUsn());
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
				cp.announceDeviceRemoved(session->getRootDevice());
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

	void UPnPControlPoint::setDeviceAddRemoveListener(AutoRef<DeviceAddRemoveListener> deviceListener) {
		this->deviceListener = deviceListener;
	}

	void UPnPControlPoint::addDevice(SSDPHeader & header) {
		Uuid uuid(header.getUsn());
		string udn = uuid.getUuid();
		InetAddress addr = header.getRemoteAddr();
		unsigned long timeout = parseCacheControlMilli(header.getCacheControl());
		if (_sessionManager.has(udn)) {
			_sessionManager[udn]->prolong(timeout);
		} else {
			AutoRef<UPnPDeviceSession> session = _sessionManager.prepareSession(udn);
			session->prolong(timeout);
			deviceBuildTaskThreadPool.setTask(AutoRef<Task>(new DeviceBuildTask(*this, session, header)));
		}
	}

	void UPnPControlPoint::removeDevice(SSDPHeader & header) {
		Uuid uuid(header.getUsn());
		string udn = uuid.getUuid();

		if (!deviceListener.nil() && !_sessionManager[udn].nil()) {
			AutoRef<UPnPDevice> device = _sessionManager[udn]->getRootDevice();
			announceDeviceRemoved(device);
		}
		
		_sessionManager.remove(udn);
	}

	AutoRef<UPnPDevice> UPnPControlPoint::buildDevice(SSDPHeader & header) {
		UPnPDeviceDeserializer deserializer;
		deserializer.setDebug(getDebug());
		return deserializer.build(Url(header.getLocation()));
	}

	void UPnPControlPoint::onDeviceBuildCompleted(AutoRef<UPnPDeviceSession> session) {
		announceDeviceAdded(session->getRootDevice());
	}
	
	void UPnPControlPoint::onDeviceBuildFailed(AutoRef<UPnPDeviceSession> session) {
		_sessionManager.remove(session->udn());
	}

	AutoRef<UPnPDevice> UPnPControlPoint::getDevice(const string & udn) {
		return _sessionManager.getDevice(udn);
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
	
	vector<AutoRef<UPnPDevice> > UPnPControlPoint::getDevices() {
		return _sessionManager.getDevices();
	}

	Url UPnPControlPoint::getBaseUrlByUdn(const string & udn) {
		return getDevice(udn)->baseUrl();
	}

	AutoRef<UPnPService> UPnPControlPoint::getServiceByUdnAndServiceType(const string & udn, const string & serviceType) {
		AutoRef<UPnPDevice> device = getDevice(udn);
		return findServiceRecursive(device, serviceType);
	}

	UPnPActionInvoker UPnPControlPoint::prepareActionInvoke(const string & udn, const string & serviceType) {
		AutoRef<UPnPDevice> device = getDevice(udn);
		AutoRef<UPnPService> service = findServiceRecursive(device, serviceType);
		if (service.nil()) {
			throw Exception("service not found / type : " + serviceType);
		}
		return UPnPActionInvoker(device->baseUrl().relativePath(service->controlUrl()));
	}

	void UPnPControlPoint::subscribe(const string & udn, const string & serviceType) {
		
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
	void UPnPControlPoint::unsubscribe(const string & udn, const string & serviceType) {
		if (eventReceiver.nil()) {
			throw Exception("event receiver is stopped");
		}

		UPnPEventSubscription subscription = eventReceiver->findSubscriptionByUdnAndServiceType(udn, serviceType);
		UPnPEventSubscriber subscriber = prepareEventSubscriber(udn, serviceType);
		subscriber.unsubscribe(subscription.sid());
	}
	UPnPEventSubscriber UPnPControlPoint::prepareEventSubscriber(const string & udn, const string & serviceType) {
		AutoRef<UPnPDevice> device = getDevice(udn);
		AutoRef<UPnPService> service = findServiceRecursive(device, serviceType);
		if (service.nil()) {
			throw Exception("service not found / type : " + serviceType);
		}
		return UPnPEventSubscriber(device->baseUrl().relativePath(service->eventSubUrl()));
	}
	AutoRef<UPnPService> UPnPControlPoint::findService(AutoRef<UPnPDevice> device, const string & serviceType) {
		if (device->hasService(serviceType)) {
			return device->getService(serviceType);
		}
		return AutoRef<UPnPService>();
	}
	AutoRef<UPnPService> UPnPControlPoint::findServiceRecursive(AutoRef<UPnPDevice> device, const string & serviceType) {
		AutoRef<UPnPService> service = findService(device, serviceType);
		if (!service.nil()) {
			return service;
		}

		for (size_t i = 0; i < device->embeddedDevices().size(); i++) {
			AutoRef<UPnPService> service = findServiceRecursive(device->embeddedDevices()[i], serviceType);
			if (!service.nil()) {
				return service;
			}
		}
		return AutoRef<UPnPService>();
	}
	AutoRef<UPnPEventReceiver> UPnPControlPoint::getEventReceiver() {
		return eventReceiver;
	}

	TimerLooperThread & UPnPControlPoint::getTimerThread() {
		return timerThread;
	}
	
	void UPnPControlPoint::collectOutdated() {
		_sessionManager.collectOutdated();
	}

	void UPnPControlPoint::addSharedDeviceList(AutoRef<SharedUPnPDeviceList> list) {
		sharedDeviceLists.push_back(list);
	}
	void UPnPControlPoint::removeSharedDeviceList(AutoRef<SharedUPnPDeviceList> list) {
		for (vector<AutoRef<SharedUPnPDeviceList> >::iterator iter = sharedDeviceLists.begin(); iter != sharedDeviceLists.end();) {
			if ((*iter) == list) {
				iter = sharedDeviceLists.erase(iter);
			} else {
				iter++;
			}
		}
	}

	void UPnPControlPoint::announceDeviceAdded(AutoRef<UPnPDevice> device) {
		if (device.nil()) {
			return;
		}
		if (!deviceListener.nil()) {
			deviceListener->onDeviceAdd(device);
		}
		for (vector<AutoRef<SharedUPnPDeviceList> >::iterator iter = sharedDeviceLists.begin(); iter != sharedDeviceLists.end(); iter++) {
			(*iter)->add_s(device);
		}
	}
	
	void UPnPControlPoint::announceDeviceRemoved(AutoRef<UPnPDevice> device) {
		if (device.nil()) {
			return;
		}
		if (!deviceListener.nil()) {
			deviceListener->onDeviceRemove(device);
		}
		for (vector<AutoRef<SharedUPnPDeviceList> >::iterator iter = sharedDeviceLists.begin(); iter != sharedDeviceLists.end(); iter++) {
			(*iter)->remove_s(device);
		}
	}

	unsigned long UPnPControlPoint::parseCacheControlMilli(const string & cacheControl) {
		try {
			MaxAge maxAge(cacheControl);
			return maxAge.second() * 1000;
		} catch (UPnPParseException e) {
			logger->loge("upnp parse exception - '" + e.toString() + "'");
			return DEFAULT_DEVICE_SESSION_TIMEOUT;
		}
	}
}
