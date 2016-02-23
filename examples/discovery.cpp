#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/Text.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>
#include <libupnp-tools/UPnPActionInvoker.hpp>

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

class Selection {
private:
	string _udn;
	string _serviceType;
	string _action;
public:
    Selection() {}
    virtual ~Selection() {}

	string & udn() {return _udn;}
	string & serviceType() {return _serviceType;}
	string & action() {return _action;}
};


int run(int argc, char *args[]) {

	Selection selection;

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
			} else if (!strncmp(buffer, "udn ", 4)) {
				selection.udn() = string(buffer + 4);
			} else if (!strcmp(buffer, "udn")) {
				cout << "UDN: " << selection.udn() << endl;
			} else if (!strncmp(buffer, "service ", 8)) {
				selection.serviceType() = string(buffer + 8);
			} else if (!strcmp(buffer, "service")) {
				cout << "Service : " << selection.serviceType() << endl;
			} else if (!strncmp(buffer, "action ", 7)) {
				selection.action() = string(buffer + 7);
			} else if (!strcmp(buffer, "action")) {
				cout << "Action : " << selection.action() << endl;
			} else if (!strcmp(buffer, "invoke")) {
				try {
					UPnPActionInvoker invoker = cp.prepareActionInvoke(selection.udn(), selection.serviceType());
					invoker.actionName() = selection.action();
					UPnPAction action = invoker.getService()->getAction(selection.action());
					if (action.name().empty()) {
						throw "Error: no action found";
					}
					vector<UPnPArgument> & arguments = action.arguments();
					for (vector<UPnPArgument>::iterator iter = arguments.begin(); iter != arguments.end(); iter++) {
						if (iter->direction() == UPnPArgument::IN_DIRECTION) {
							char param[1024] = {0,};
							prompt(iter->name() + " : ", param, sizeof(param));
							invoker.inParams()[iter->name()] = string(param);
						}
					}
					invoker.invoke();
					map<string, string> & params = invoker.outParams();
					for (map<string, string>::iterator iter = params.begin(); iter != params.end(); iter++) {
						string name = iter->first;
						string & value = iter->second;

						cout << " - " << name << " := " << value << endl;
					}
				} catch (const char * e) {
					cout << "Error: " << e << endl;
				}
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
