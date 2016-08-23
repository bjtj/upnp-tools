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

	vector<AutoRef<UPnPDevice> > SharedUPnPDeviceList::list() {
		return _devices;
	}
	vector<AutoRef<UPnPDevice> > SharedUPnPDeviceList::list_s() {
		lock();
		vector<AutoRef<UPnPDevice> > ret = list();
		unlock();
		return ret;
	}
	AutoRef<UPnPDevice> SharedUPnPDeviceList::findByUdn(const string & udn) {
		AutoRef<UPnPDevice> ret;
		for (vector<AutoRef<UPnPDevice> >::iterator iter = _devices.begin(); iter != _devices.end(); iter++) {
			if ((*iter)->getUdn() == udn) {
				ret = *iter;
			}
		}
		return ret;
	}
	AutoRef<UPnPDevice> SharedUPnPDeviceList::findByUdn_s(const string & udn) {
		lock();
		AutoRef<UPnPDevice> ret = findByUdn(udn);
		unlock();
		return ret;
	}
	void SharedUPnPDeviceList::add(AutoRef<UPnPDevice> device) {
		_devices.push_back(device);
	}
	void SharedUPnPDeviceList::add_s(AutoRef<UPnPDevice> device) {
		lock();
		add(device);
		unlock();
	}
	void SharedUPnPDeviceList::remove(AutoRef<UPnPDevice> device) {
		for (vector<AutoRef<UPnPDevice> >::iterator iter = _devices.begin(); iter != _devices.end();) {
			if ((*iter) == device) {
				iter = _devices.erase(iter);
			} else {
				iter++;
			}
		}
	}
	void SharedUPnPDeviceList::remove_s(AutoRef<UPnPDevice> device) {
		lock();
		remove(device);
		unlock();
	}
	size_t SharedUPnPDeviceList::size() {
		return _devices.size();
	}
	size_t SharedUPnPDeviceList::size_s() {
		lock();
		size_t ret = _devices.size();
		unlock();
		return ret;
	}
}
