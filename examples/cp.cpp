#include <iostream>
#include <string>
#include <map>
#include <liboslayer/Text.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>

using namespace std;
using namespace UTIL;
using namespace UPNP;

vector<UPnPDevice> devices;

class MyDeviceAddRemoveHandle : public DeviceAddRemoveListener {
private:
public:
	MyDeviceAddRemoveHandle() {}
	virtual ~MyDeviceAddRemoveHandle() {}

	virtual void onDeviceAdded(UPnPControlPoint & cp, UPnPDevice & device) {
		// cout << " ++ " << device.getUdn() << " ** " << device.getFriendlyName() << endl;
		devices.push_back(device);
	}
	virtual void onDeviceRemoved(UPnPControlPoint & cp, UPnPDevice & device) {
		// cout << " -- " << device.getUdn() << " ** " << device.getFriendlyName() << endl;
		for (vector<UPnPDevice>::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			if (!iter->getUdn().compare(device.getUdn())) {
				devices.erase(iter);
				return;
			}
		}
	}
};

class MyInvokeActionResponseListener : public InvokeActionResponseListener {
	virtual void onActionResponse(ID_TYPE id, const UPnPActionRequest & actionRequest, const UPnPActionResponse & response) {
		cout << "#RES : " << actionRequest.getActionName() << endl;
		vector<string> names = response.getParameterNames();
		for (size_t i = 0; i < names.size(); i++) {
			string & name = names[i];
			string value = response.getParameter(name);
			cout << " - " << name << " : " << value << endl;
		}
	}
};

static int s_cmd_handler(const char * cmd, UPnPControlPoint & cp) {

	if (!strcmp(cmd, "quit") || !strcmp(cmd, "q")) {
		return -1;
	}

	if (!strcmp(cmd, "l") || !strcmp(cmd, "list")) {

		cout << "== DEVICE LIST (count: " << devices.size() << ") ==" << endl;
		for (size_t i = 0; i < devices.size(); i++) {
			UPnPDevice & device = devices[i];
			cout << " - UDN: " << device.getUdn() << endl;
			cout << "  > Friendly Name: " << device.getFriendlyName() << endl;
		}
		cout << endl;

		return 0;
	}

	if (!strcmp(cmd, "m") || !strcmp(cmd, "scan")) {
		cp.sendMsearch("upnp:rootdevice");
		return 0;
	}
    
    if (!strcmp(cmd, "a") || !strcmp(cmd, "action")) {
        // cp.invokeAction(targetService, "GetSystemUpdateID", UPnPActionParameters());
    }

	return 0;
}

size_t readline(char * buffer, size_t max) {
	if (fgets(buffer, (int)max - 1, stdin)) {
		buffer[strlen(buffer) - 1] = 0;
		return strlen(buffer);
	}
	return 0;
}

static void s_test_cp() {

	bool done = false;
	char buffer[1024] = {0,};

	
    UPnPControlPoint cp;
	UPnPControlPointSsdpNotifyFilter filter;
	filter.addFilterType("upnp:rootdevice");
	cp.setFilter(filter);

	MyDeviceAddRemoveHandle listener;
	cp.setDeviceAddRemoveListener(&listener);
    
    MyInvokeActionResponseListener actionResponseListener;
    cp.setInvokeActionResponseListener(&actionResponseListener);

    cp.startAsync();
    
    cout << "cp - start" << endl;
    
	while (!done) {
		readline(buffer, sizeof(buffer));
		if (s_cmd_handler(buffer, cp) < 0) {
			done = true;
			break;
		}
	}

    cp.stop();
    
    cout << "cp - finish" << endl;
}

/**
 * main
 */
int main(int argc, char *args[]) {
	s_test_cp();
    return 0;
}
