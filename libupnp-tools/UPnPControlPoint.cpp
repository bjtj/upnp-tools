#include "UPnPControlPoint.hpp"
#include "Uuid.hpp"
#include <iostream>
#include "NetworkUtil.hpp"

namespace UPNP {

	using namespace UTIL;
	using namespace HTTP;
	using namespace SSDP;
	using namespace std;
	using namespace OS;

	class MySSDPHandler : public SSDPEventHandler {
	private:
		UPnPControlPoint * cp;
	public:
		MySSDPHandler(UPnPControlPoint * cp) : cp(cp) {}
		virtual ~MySSDPHandler() {}

		void setControlPoint(UPnPControlPoint * cp) {
			this->cp = cp;
		}
	
		virtual bool filter(SSDPHeader & header) {
			return true;
		}
		virtual void onMsearch(SSDPHeader & header) {
		}
		virtual void onNotify(SSDPHeader & header) {
			OS::InetAddress addr = header.getRemoteAddr();
			if (cp) {
				if (header.isNotifyAlive()) {
					cp->addDevice(header);
				} else {
					cp->removeDevice(header);
				}
			}
		}
		virtual void onMsearchResponse(SSDPHeader & header) {
			OS::InetAddress addr = header.getRemoteAddr();

			Uuid uuid(header.getUsn());

			if (cp) {
				cp->addDevice(header);
			}
		}
	};

	
	UPnPControlPoint::UPnPControlPoint(UPnPControlPointConfig & config) : config(config), ssdpHandler(new MySSDPHandler(this)), notificationServer(NULL) {
	}
	
	UPnPControlPoint::~UPnPControlPoint() {
	}
		
	void UPnPControlPoint::startAsync() {
		ssdpServer.setSSDPEventHandler(ssdpHandler);
		ssdpServer.startAsync();

		if (!notificationServer) {
			UPnPNotificationServerConfig notificationServerConfig(config.getIntegerProperty("listen.port"));
			notificationServer = new UPnPNotificationServer(notificationServerConfig);
			notificationServer->startAsync();
		}
	}

	void UPnPControlPoint::stop() {
		if (notificationServer) {
			notificationServer->stop();
			delete notificationServer;
			notificationServer = NULL;
		}
		
		ssdpServer.stop();
	}

	void UPnPControlPoint::setDeviceAddRemoveListener(AutoRef<DeviceAddRemoveListener> deviceListener) {
		this->deviceListener = deviceListener;
	}

	void UPnPControlPoint::addDevice(SSDPHeader & header) {
		Uuid uuid(header.getUsn());
		string udn = uuid.getUuid();
		if (!_sessionManager.has(udn)) {
			AutoRef<UPnPSession> session = _sessionManager.prepareSession(udn);
			try {
				session->buildDevice(header);
				if (!deviceListener.nil()) {
					deviceListener->onDeviceAdd(session->getRootDevice());
				}
			} catch (Exception & e) {
				cout << e.getMessage() << endl;
				_sessionManager.remove(udn);
				return;
			}
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

	AutoRef<UPnPDevice> UPnPControlPoint::getDevice(const string & udn) {
		AutoRef<UPnPSession> session = _sessionManager[udn];
		if (!session.nil()) {
			return session->getRootDevice();
		}
		throw OS::Exception("not found device / name : " + udn, -1, 0);
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
		return device->getService(serviceType);
	}

	UPnPActionInvoker UPnPControlPoint::prepareActionInvoke(const string & udn, const string & serviceType) {
		AutoRef<UPnPDevice> device = getDevice(udn);
		AutoRef<UPnPService> service = device->getService(serviceType);
		return UPnPActionInvoker(device->baseUrl().relativePath(service->getControlUrl()));
	}

	void UPnPControlPoint::subscribe(const string & udn, const string & serviceType) {
		if (!notificationServer) {
			throw Exception("notification server is not working");
		}

		OS::InetAddress addr = NetworkUtil::selectDefaultAddress();

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
		AutoRef<UPnPService> service = device->getService(serviceType);
		return UPnPEventSubscriber(device->baseUrl().relativePath(service->getEventSubUrl()));
	}

	UPnPNotificationServer * UPnPControlPoint::getNotificationServer() {
		return notificationServer;
	}
}
