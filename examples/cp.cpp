#include <iostream>
#include <string>
#include <map>
#include <liboslayer/Text.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>

using namespace std;
using namespace UTIL;
using namespace UPNP;

UPnPService targetService;

class MyDeviceAddRemoveHandle : public DeviceAddRemoveListener {
private:
public:
	MyDeviceAddRemoveHandle() {}
	virtual ~MyDeviceAddRemoveHandle() {}

	virtual void onDeviceAdded(UPnPControlPoint & cp, UPnPDevice & device) {
		cout << "[Device Added] udn: " << device["UDN"] << " - " << device["friendlyName"] << endl;
		UPnPService service = device.getService("urn:schemas-upnp-org:service:ContentDirectory:1");
        
		if (!service.empty()) {
            targetService = service;
            cout << "invoke action" << endl;
			cp.invokeAction(service, "GetSystemUpdateID", UPnPActionParameters());
		}
	}
	virtual void onDeviceRemoved(UPnPControlPoint & cp, UPnPDevice & device) {
        cout << "[Device Removed] udn: " << device["UDN"] << " - " << device["friendlyName"] << endl;
	}
};

class MyInvokeActionResponseListener : public InvokeActionResponseListener {
	virtual void onActionResponse(ID_TYPE id, const UPnPActionRequest & actionRequest, const UPnPActionParameters & out) {
        cout << "ActionResponse> Action name: " << actionRequest.actionName << endl;
	}
};

static int s_cmd_handler(const char * cmd, UPnPControlPoint & cp) {

	if (!strcmp(cmd, "quit") || !strcmp(cmd, "q")) {
		return -1;
	}

	if (!strcmp(cmd, "m") || !strcmp(cmd, "scan")) {
		cp.sendMsearch("upnp:rootdevice");
		return 0;
	}
    
    if (!strcmp(cmd, "a") || !strcmp(cmd, "action")) {
        cp.invokeAction(targetService, "GetSystemUpdateID", UPnPActionParameters());
    }

	return 0;
}

size_t readline(char * buffer, size_t max) {
	fgets(buffer, (int)max - 1, stdin);
	buffer[strlen(buffer) - 1] = 0;
	return strlen(buffer);
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
