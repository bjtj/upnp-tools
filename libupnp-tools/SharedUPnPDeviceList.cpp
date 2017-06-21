#include "SharedUPnPDeviceList.hpp"

namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace UTIL;

	SharedUPnPDeviceList::SharedUPnPDeviceList() : sem(1) {
	}
	SharedUPnPDeviceList::~SharedUPnPDeviceList() {
	}

	void SharedUPnPDeviceList::lock() {
		sem.wait();
	}
	void SharedUPnPDeviceList::unlock() {
		sem.post();
	}
	vector< AutoRef<UPnPDevice> > SharedUPnPDeviceList::list() {
		return _devices;
	}
	void SharedUPnPDeviceList::list(vector<AutoRef<UPnPDevice> > & vec) {
		lock();
		vec.insert(vec.end(), _devices.begin(), _devices.end());
		unlock();
	}
	AutoRef<UPnPDevice> SharedUPnPDeviceList::findByUdn(const UDN & udn) {
		AutoRef<UPnPDevice> ret;
		lock();
		for (vector<AutoRef<UPnPDevice> >::iterator iter = _devices.begin(); iter != _devices.end(); iter++) {
			if ((*iter)->getUdn() == udn) {
				ret = *iter;
			}
		}
		unlock();
		return ret;
	}
	void SharedUPnPDeviceList::add(AutoRef<UPnPDevice> device) {
		lock();
		_devices.push_back(device);
		unlock();
	}
	void SharedUPnPDeviceList::remove(AutoRef<UPnPDevice> device) {
		lock();
		for (vector<AutoRef<UPnPDevice> >::iterator iter = _devices.begin(); iter != _devices.end();) {
			if ((*iter) == device) {
				iter = _devices.erase(iter);
			} else {
				iter++;
			}
		}
		unlock();
	}
	size_t SharedUPnPDeviceList::size() {
		return _devices.size();
	}
}
