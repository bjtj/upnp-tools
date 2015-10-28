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
		cout << "[Device Added] udn: " << device["UDN"] << " - " << device["friendlyName"] << endl;
	}
	virtual void onDeviceRemove(UPnPDevice & device) {
        cout << "[Device Removed] udn: " << device["UDN"] << " - " << device["friendlyName"] << endl;
	}
};

static int s_cmd_handler(const char * cmd, UPnPControlPoint & cp) {

	if (!strcmp(cmd, "quit") || !strcmp(cmd, "q")) {
		return -1;
	}

	if (!strcmp(cmd, "msearch") || !strcmp(cmd, "scan")) {
		cp.sendMsearch();
		return 0;
	}

	return 0;
}

static void s_test_cp() {

	bool done = false;
	char buffer[1024] = {0,};

	vector<string> filter;
	filter.push_back("upnp:rootdevice");
    UPnPControlPoint cp(1900, "upnp:rootdevice", filter);

	MyDeviceAddRemoveHandle listener;
	cp.setOnDeviceAddRemoveListener(&listener);

    cp.startAsync();
    
	while (!done) {
		printf(">>");
		fgets(buffer, sizeof(buffer) - 1, stdin);
		int len = strlen(buffer);
		buffer[len - 1] = 0;
		if (s_cmd_handler(buffer, cp) < 0) {
			done = true;
			break;
		}
	}

    cp.stop();
}

/**
 * main
 */
int main(int argc, char *args[]) {
	s_test_cp();
    return 0;
}
