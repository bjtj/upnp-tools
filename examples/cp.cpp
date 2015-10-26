#include <iostream>
#include <string>
#include <map>
#include <liboslayer/Text.hpp>

#include <libupnp-tools/UPnPControlPoint.hpp>

using namespace std;
using namespace UTIL;
using namespace UPNP;

class MyDeviceAddRemoveHandle : public OnDeviceAddRemoveListener {
private:
public:
	MyDeviceAddRemoveHandle() {}
	virtual ~MyDeviceAddRemoveHandle() {}

	virtual void onDeviceAdd(UPnPDevice & device) {
		cout << "[Device Added] udn: " << device["UDN"] << endl;
	}
	virtual void onDeviceRemove(UPnPDevice & device) {
	}
};

static void s_test_cp() {
    UPnPControlPoint cp(1900, "ssdp:all");

	MyDeviceAddRemoveHandle listener;
	cp.setOnDeviceAddRemoveListener(&listener);

    cp.startAsync();
    
    getchar();
    
    cp.stop();
}

/**
 * main
 */
int main(int argc, char *args[]) {
	s_test_cp();
    return 0;
}
