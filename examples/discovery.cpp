#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/Text.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>

using namespace std;
using namespace UTIL;
using namespace UPNP;

int run(int argc, char *args[]);

int main(int argc, char *args[]) {
	return run(argc, args);
}

size_t readline(char * buffer, size_t max) {
    if (fgets(buffer, (int)max - 1, stdin)) {
		buffer[strlen(buffer) - 1] = 0;
		return strlen(buffer);
	}
    return 0;
}

size_t prompt(const string & msg, char * buffer, size_t max) {
	cout << msg;
	return readline(buffer, max);
}

void printList(UPnPSessionManager & sessionManager) {
	cout << " == Device List (" << sessionManager.getUdnS().size() << ") ==" << endl;
	vector<string> lst = sessionManager.getUdnS();
	size_t i = 0;
	for (vector<string>::iterator iter = lst.begin(); iter != lst.end(); iter++, i++) {
		AutoRef<UPnPSession> session = sessionManager[*iter];
		cout << "[" << i << "] " << session->toString() << endl;
	}
}

void selectDeviceByIndex(UPnPSessionManager & sessionManager, size_t idx) {
	if (idx >= sessionManager.size()) {
		return;
	}

	string udn = sessionManager.getUdnS()[idx];
	AutoRef<UPnPSession> session = sessionManager[udn];

	cout << session->toString() << endl;
}

class MyDeviceListener : public DeviceAddRemoveListener {
private:
public:
    MyDeviceListener() {}
    virtual ~MyDeviceListener() {}

	virtual void onDeviceAdd(AutoRef<UPnPDevice> device) {
		cout << " ** Added: " << device->getFriendlyName() << endl;
	}

	virtual void onDeviceRemove(AutoRef<UPnPDevice> device) {
		cout << " ** Removed: " << device->getFriendlyName() << endl;
	}
};


int run(int argc, char *args[]) {

	UPnPControlPoint cp;

	cp.setDeviceAddRemoveListener(AutoRef<DeviceAddRemoveListener>(new MyDeviceListener));
	cp.startAsync();

	while (1) {
		char buffer[1024] = {0,};
		if (readline(buffer, sizeof(buffer)) > 0) {
			if (!strcmp(buffer, "q")) {
				break;
			} else if (!strcmp(buffer, "clear")) {
				cp.clearDevices();
			} else if (string(buffer).find_first_not_of("0123456789") == string::npos) {
				int idx = Text::toInt(buffer);
				cout << "idx : " << idx << endl;
				selectDeviceByIndex(cp.sessionManager(), (size_t)idx);
			} else {
				cout << " ** Searching... **" << endl;
				cp.sendMsearchAndWait(buffer, 3);
				cout << endl << " ** Searching Done **" << endl;
			}
		} else {
			printList(cp.sessionManager());
		}
	}

	cp.stop();
    
    return 0;
}
