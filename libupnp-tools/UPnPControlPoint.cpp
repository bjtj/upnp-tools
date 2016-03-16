#include "UPnPControlPoint.hpp"
#include "Uuid.hpp"
#include <iostream>

namespace UPNP {

	using namespace UTIL;
	using namespace HTTP;
	using namespace SSDP;
	using namespace std;

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
			if (header.isNotifyAlive()) {
				if (cp) {
					cp->addDevice(header);
				}
			
			} else {
				if (cp) {
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

	
	UPnPControlPoint::UPnPControlPoint() : ssdpHandler(new MySSDPHandler(this)) {
	}
	UPnPControlPoint::~UPnPControlPoint() {
	}
		
	void UPnPControlPoint::startAsync() {
		ssdpServer.setSSDPEventHandler(ssdpHandler);
		ssdpServer.startAsync(100);
	}

	void UPnPControlPoint::stop() {
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
			session->buildDevice(header);
			AutoRef<UPnPDevice> device = session->getRootDevice();
			if (device.nil()) {
				_sessionManager.remove(udn);
				return;
			}

			if (!deviceListener.nil()) {
				deviceListener->onDeviceAdd(device);
			}
		}
	}

	void UPnPControlPoint::removeDevice(SSDPHeader & header) {
		Uuid uuid(header.getUsn());
		string udn = uuid.getUuid();

		if (!deviceListener.nil()) {
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
	
	void UPnPControlPoint::sendMsearchAndWait(const std::string & target, unsigned long timeoutSec) {
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

	UPnPActionInvoker UPnPControlPoint::prepareActionInvoke(const std::string & udn, const std::string & serviceType) {
		AutoRef<UPnPDevice> device = getDevice(udn);
		AutoRef<UPnPService> service = device->getService(serviceType);
		return UPnPActionInvoker(device->baseUrl().relativePath(service->getControlUrl()));
	}
}
