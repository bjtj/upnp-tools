#include "UPnPControlPoint.hpp"
#include "Uuid.hpp"
#include "NetworkUtil.hpp"

namespace UPNP {

	using namespace UTIL;
	using namespace HTTP;
	using namespace SSDP;
	using namespace std;
	using namespace OS;

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
			cp.clearOudatedSessions();
		}
	};

	/**
	 * @brief
	 */
	class DeviceBuildTask : public Task {
	private:
		UPnPControlPoint & cp;
		AutoRef<UPnPSession> session;
		SSDPHeader header;
	public:
		DeviceBuildTask(UPnPControlPoint & cp, AutoRef<UPnPSession> session, SSDPHeader & header) : cp(cp), session(session), header(header) {}
		virtual ~DeviceBuildTask() {}

		virtual void doTask() {
			try {
				session->buildDevice(header);
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
	class ControlPointSSDPHandler : public SSDPEventHandler {
	private:
		UPnPControlPoint * cp;
	public:
		ControlPointSSDPHandler(UPnPControlPoint * cp) : cp(cp) {}
		virtual ~ControlPointSSDPHandler() {}

		void setControlPoint(UPnPControlPoint * cp) {
			this->cp = cp;
		}
	
		virtual bool filter(SSDPHeader & header) {
			return true;
		}
		virtual void onMsearch(SSDPHeader & header) {
		}
		virtual void onNotify(SSDPHeader & header) {
			InetAddress addr = header.getRemoteAddr();
			if (cp) {
				if (header.isNotifyAlive()) {
					cp->addDevice(header);
				} else {
					cp->removeDevice(header);
				}
			}
		}
		virtual void onMsearchResponse(SSDPHeader & header) {
			InetAddress addr = header.getRemoteAddr();

			Uuid uuid(header.getUsn());

			if (cp) {
				cp->addDevice(header);
			}
		}
	};

	// 
	
	UPnPControlPoint::UPnPControlPoint(UPnPControlPointConfig & config) : config(config), ssdpHandler(new ControlPointSSDPHandler(this)), notificationServer(NULL), started(false), deviceBuildTaskThreadPool(10) {
	}
	
	UPnPControlPoint::~UPnPControlPoint() {
	}
		
	void UPnPControlPoint::startAsync() {

		if (started) {
			return;
		}
		
		ssdpServer.addSSDPEventHandler(ssdpHandler);
		ssdpServer.startAsync();

		if (!notificationServer) {
			UPnPNotificationServerConfig notificationServerConfig(config.getIntegerProperty("listen.port"));
			notificationServer = new UPnPNotificationServer(notificationServerConfig);
			notificationServer->startAsync();
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
		timerThread.join();
		
		if (notificationServer) {
			notificationServer->stop();
			delete notificationServer;
			notificationServer = NULL;
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
		if (!_sessionManager.has(udn)) {
			AutoRef<UPnPSession> session = _sessionManager.prepareSession(udn);
			deviceBuildTaskThreadPool.setTask(AutoRef<Task>(new DeviceBuildTask(*this, session, header)));
		} else {
			_sessionManager[udn]->prolong(header.getCacheControl());
		}
	}

	void UPnPControlPoint::removeDevice(SSDPHeader & header) {
		Uuid uuid(header.getUsn());
		string udn = uuid.getUuid();

		if (!deviceListener.nil() && !_sessionManager[udn].nil()) {
			AutoRef<UPnPDevice> device = _sessionManager[udn]->getRootDevice();
			if (!device.nil()) {
				deviceListener->onDeviceRemove(device);
			}
		}
		
		_sessionManager.remove(udn);
	}

	void UPnPControlPoint::onDeviceBuildCompleted(AutoRef<UPnPSession> session) {
		if (!deviceListener.nil()) {
			deviceListener->onDeviceAdd(session->getRootDevice());
		}
	}
	void UPnPControlPoint::onDeviceBuildFailed(AutoRef<UPnPSession> session) {
		_sessionManager.remove(session->udn());
	}

	AutoRef<UPnPDevice> UPnPControlPoint::getDevice(const string & udn) {
		AutoRef<UPnPSession> session = _sessionManager[udn];
		if (!session.nil()) {
			return session->getRootDevice();
		}
		throw Exception("not found device / name : " + udn, -1, 0);
	}

	void UPnPControlPoint::clearDevices() {
		_sessionManager.clear();
	}
	
	void UPnPControlPoint::sendMsearchAndWait(const string & target, unsigned long timeoutSec) {
		ssdpServer.sendMsearchAndGather(target, timeoutSec);
	}

	UPnPSessionManager & UPnPControlPoint::sessionManager() {
		return _sessionManager;
	}

	Url UPnPControlPoint::getBaseUrlWithUdn(const string & udn) {
		return getDevice(udn)->baseUrl();
	}

	AutoRef<UPnPService> UPnPControlPoint::getServiceWithUdnAndServiceType(const string & udn, const string & serviceType) {
		AutoRef<UPnPDevice> device = getDevice(udn);
		return findServiceRecursive(device, serviceType);
	}

	UPnPActionInvoker UPnPControlPoint::prepareActionInvoke(const string & udn, const string & serviceType) {
		AutoRef<UPnPDevice> device = getDevice(udn);
		AutoRef<UPnPService> service = findServiceRecursive(device, serviceType);
		if (service.nil()) {
			throw Exception("service not found / type : " + serviceType);
		}
		return UPnPActionInvoker(device->baseUrl().relativePath(service->getControlUrl()));
	}

	void UPnPControlPoint::subscribe(const string & udn, const string & serviceType) {
		if (!notificationServer) {
			throw Exception("notification server is not working");
		}

		InetAddress addr = NetworkUtil::selectDefaultAddress();

		UPnPEventSubscriber subscriber = prepareEventSubscriber(udn, serviceType);
		UPnPEventSubscribeRequest request(notificationServer->getCallbackUrl(addr.getHost()), 300);
		UPnPEventSubscribeResponse response = subscriber.subscribe(request);

		UPnPEventSubscription subscription(response.sid());
		subscription.udn() = udn;
		subscription.serviceType() = serviceType;
		notificationServer->addSubscription(subscription);
		
	}
	void UPnPControlPoint::unsubscribe(const string & udn, const string & serviceType) {
		if (!notificationServer) {
			throw Exception("notification server is not working");
		}

		UPnPEventSubscription subscription = notificationServer->findSubscriptionWithUdnAndServiceType(udn, serviceType);
		UPnPEventSubscriber subscriber = prepareEventSubscriber(udn, serviceType);
		subscriber.unsubscribe(subscription.sid());
	}
	UPnPEventSubscriber UPnPControlPoint::prepareEventSubscriber(const string & udn, const string & serviceType) {
		AutoRef<UPnPDevice> device = getDevice(udn);
		AutoRef<UPnPService> service = findServiceRecursive(device, serviceType);
		if (service.nil()) {
			throw Exception("service not found / type : " + serviceType);
		}
		return UPnPEventSubscriber(device->baseUrl().relativePath(service->getEventSubUrl()));
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

		for (size_t i = 0; i < device->devices().size(); i++) {
			AutoRef<UPnPService> service = findServiceRecursive(device->devices()[i], serviceType);
			if (!service.nil()) {
				return service;
			}
		}
		return AutoRef<UPnPService>();
	}
	UPnPNotificationServer * UPnPControlPoint::getNotificationServer() {
		return notificationServer;
	}

	UTIL::TimerLooperThread & UPnPControlPoint::getTimerThread() {
		return timerThread;
	}
	void UPnPControlPoint::clearOudatedSessions() {
		vector<AutoRef<UPnPSession> > sessions = _sessionManager.getOutdatedSessions();
		for (vector<AutoRef<UPnPSession> >::iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
			AutoRef<UPnPDevice> device = (*iter)->getRootDevice();
			deviceListener->onDeviceRemove(device);
			_sessionManager.remove((*iter)->udn());
		}
	}
}
