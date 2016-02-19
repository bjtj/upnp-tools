#include "UPnPControlPoint.hpp"
#include "Uuid.hpp"
#include <iostream>

namespace UPNP {

	const static bool ENABLE_LOG = false;

	using namespace UTIL;
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
			OS::InetAddress addr = header.getRemoteAddr();
			if (ENABLE_LOG) {
				printf("> MSEARCH / ST: %s (%s:%d)\n",
					   header.getSt().c_str(), addr.getHost().c_str(), addr.getPort());
			}
		}
		virtual void onNotify(SSDPHeader & header) {
			OS::InetAddress addr = header.getRemoteAddr();
			if (header.isNotifyAlive()) {
				if (ENABLE_LOG) {
					printf("> NOTIFY / alive :: URL: %s (%s:%d)\n",
						   header.getLocation().c_str(), addr.getHost().c_str(), addr.getPort());
				}

				if (cp) {
					cp->addDevice(header);
				}
			
			} else {
				if (ENABLE_LOG) {
					printf("> NOTIFY / byebye :: %s (%s:%d)\n",
						   header.getNt().c_str(), addr.getHost().c_str(), addr.getPort());
				}
				
				if (cp) {
					cp->removeDevice(header);
				}
			}
		}
		virtual void onMsearchResponse(SSDPHeader & header) {
			OS::InetAddress addr = header.getRemoteAddr();

			Uuid uuid(header.getUsn());

			if (ENABLE_LOG) {
				printf("> RESP / USN: %s / ST: %s / URL: %s (%s:%d)\n",
					   header.getUsn().c_str(), header.getSt().c_str(),
					   header.getLocation().c_str(), addr.getHost().c_str(), addr.getPort());
			}

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

			if (ENABLE_LOG) {
				cout << " ++ DEVICE : " << device->getUdn() << " - " << device->getFriendlyName() << endl;
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

		if (ENABLE_LOG) {
			cout << " -- DEVICE : " << udn << endl;
		}
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
}
