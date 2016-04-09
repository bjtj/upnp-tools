#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/Text.hpp>
#include <liboslayer/XmlParser.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>
#include <libupnp-tools/UPnPActionInvoker.hpp>
#include <libupnp-tools/UPnPActionRequest.hpp>
#include <libupnp-tools/UPnPActionResponse.hpp>
#include <libupnp-tools/NetworkUtil.hpp>
#include <libupnp-tools/UPnPEventSubscriber.hpp>
#include <libupnp-tools/UPnPNotificationServer.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>

using namespace std;
using namespace UTIL;
using namespace UPNP;
using namespace HTTP;
using namespace XML;

class MyNotifyListener : public UPnPNotificationListener {
private:
public:
    MyNotifyListener() {}
    virtual ~MyNotifyListener() {}
	virtual void onNotify(UPnPNotify & notify) {
		cout << "Notify : " << notify.sid() << endl;
		vector<string> names = notify.propertyNames();
		for (vector<string>::iterator iter = names.begin(); iter != names.end(); iter++) {
			string value = notify[*iter];
			cout << " - " << *iter << " : " << value << endl;
		}
	}
};


static int run(int argc, char *args[]);

int main(int argc, char *args[]) {

	try {
		return run(argc, args);
	} catch (const char * e) {
		cerr << e << endl;
		return 1;
	} catch (const string & e) {
		cerr << e << endl;
		return 1;
	} catch (OS::Exception & e) {
		cerr << e.getMessage() << endl;
		return 1;
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

size_t prompt(const string & msg, char * buffer, size_t max) {
	cout << msg;
	return readline(buffer, max);
}

class UPnPSessionPrinter {
private:
	UPnPSession & session;
public:
    UPnPSessionPrinter(UPnPSession & session) : session(session) {}
    virtual ~UPnPSessionPrinter() {}
	
	string toString() {
		if (session.getRootDevice().nil()) {
			return "(null)";
		}
		return toString(*(session.getRootDevice()), 0);
	}

	string toString(UPnPDevice & device, int depth) {
		string str;

		str.append(depth, ' ');
		if (depth > 0) { str.append(" - "); }
		str.append(device.getUdn() + " (" + device.getFriendlyName() + ")");
		str.append(" - " + Text::toString(session.lifetimeFromLastUpdate()) + " ms.");

		vector<AutoRef<UPnPService> > services = device.services();
		for (vector<AutoRef<UPnPService> >::iterator iter = services.begin(); iter != services.end(); iter++) {
			str.append("\n");
			str.append(depth, ' ');
			str.append(" ** " + (*iter)->getServiceType());

			vector<UPnPAction> actions = (*iter)->actions();
			for (vector<UPnPAction>::iterator aiter = actions.begin(); aiter != actions.end(); aiter++) {
				str.append("\n");
				str.append(depth, ' ');
				str.append("  - " + (*aiter).name());
			}
		}
			
		vector<AutoRef<UPnPDevice> > & devices = device.devices();
		for (vector<AutoRef<UPnPDevice> >::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			str.append("\n");
			str.append(toString(*(*iter), depth + 1));
		}
		return str;
	}

	string toBriefString() {
		if (session.getRootDevice().nil()) {
			return "";
		}
		return toBriefString(*(session.getRootDevice()), 0);
	}

	string toBriefString(UPnPDevice & device, int depth) {
		string str;

		str.append(depth, ' ');
		if (depth > 0) { str.append(" - "); }
		str.append(device.getUdn() + " (" + device.getFriendlyName() + ")");
		str.append(" - " + Text::toString(session.lifetimeFromLastUpdate()) + " ms.");

		vector<AutoRef<UPnPService> > services = device.services();
		for (vector<AutoRef<UPnPService> >::iterator iter = services.begin(); iter != services.end(); iter++) {
			str.append("\n");
			str.append(depth, ' ');
			str.append(" ** " + (*iter)->getServiceType());
		}
			
		vector<AutoRef<UPnPDevice> > & devices = device.devices();
		for (vector<AutoRef<UPnPDevice> >::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			str.append("\n");
			str.append(toBriefString(*(*iter), depth + 1));
		}
		return str;
	}
};




void printList(UPnPSessionManager & sessionManager) {
	cout << " == Device List (" << sessionManager.getUdnS().size() << ") ==" << endl;
	vector<string> lst = sessionManager.getUdnS();
	size_t i = 0;
	for (vector<string>::iterator iter = lst.begin(); iter != lst.end(); iter++, i++) {
		AutoRef<UPnPSession> session = sessionManager[*iter];
		UPnPSessionPrinter printer(*session);
		cout << "[" << i << "] " << printer.toBriefString() << endl;
	}
}

void selectDeviceByIndex(UPnPSessionManager & sessionManager, size_t idx) {
	if (idx >= sessionManager.size()) {
		return;
	}

	string udn = sessionManager.getUdnS()[idx];
	AutoRef<UPnPSession> session = sessionManager[udn];

	UPnPSessionPrinter printer(*session);
	cout << printer.toString() << endl;
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

	UPnPControlPointConfig config(9998);
	UPnPControlPoint cp(config);

	cp.setDeviceAddRemoveListener(AutoRef<DeviceAddRemoveListener>(new MyDeviceListener));
	cp.startAsync();

	cp.getNotificationServer()->addNotificationListener(AutoRef<UPnPNotificationListener>(new MyNotifyListener));

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
					AutoRef<UPnPService> service = cp.getServiceWithUdnAndServiceType(selection.udn(), selection.serviceType());
					UPnPAction action = service->getAction(selection.action());
					if (action.name().empty()) {
						throw "Error: no action found";
					}
					vector<UPnPArgument> & arguments = action.arguments();
					UPnPActionRequest request;
					request.serviceType() = service->getServiceType();
					request.action() = action;
					for (vector<UPnPArgument>::iterator iter = arguments.begin(); iter != arguments.end(); iter++) {
						if (iter->direction() == UPnPArgument::IN_DIRECTION) {
							string allows = "";
							UPnPStateVariable sv = service->getStateVariable(iter->stateVariableName());
							if (sv.hasAllowedValues()) {
								for (vector<string>::iterator ai = sv.allowedValueList().begin(); ai != sv.allowedValueList().end(); ai++) {
									if (allows.length() > 0) {
										allows.append(", ");
									}
									allows.append(*ai);
								}
								allows = " [" + allows + "]";
							}
							char param[1024] = {0,};
							prompt(iter->name() + allows + " : ", param, sizeof(param));
							request[iter->name()] = string(param);
						}
					}
					UPnPActionResponse response = invoker.invoke(request);
					map<string, string> & params = response.parameters();
					for (map<string, string>::iterator iter = params.begin(); iter != params.end(); iter++) {
						string name = iter->first;
						string & value = iter->second;

						cout << " - " << name << " := " << value << endl;
					}
				} catch (const char * e) {
					cout << "Error: " << e << endl;
				} catch (const string & e) {
					cout << "Error: " << e << endl;
				} catch (OS::Exception & e) {
					cout << "Error: " << e.getMessage() << endl;
				}
			} else if (!strcmp(buffer, "subs")) {
				// TODO: subscription list
			} else if (!strcmp(buffer, "sub")) {

				if (selection.udn().empty() || selection.serviceType().empty()) {
					throw "Error: select udn and sevice first";
				}

				cout << "Subscribe - " << selection.udn() << " .. " << selection.serviceType() << endl;
				cp.subscribe(selection.udn(), selection.serviceType());

			} else if (!strcmp(buffer, "unsub")) {

				if (selection.udn().empty() || selection.serviceType().empty()) {
					throw "Error: select udn and sevice first";
				}

				cout << "Unsubscribe - " << selection.udn() << " .. " << selection.serviceType() << endl;
				cp.unsubscribe(selection.udn(), selection.serviceType());

			} else if (!strcmp(buffer, "dump")) {

				if (selection.udn().empty()) {
					throw "Error: select udn first";
				}

				Url url = cp.getBaseUrlWithUdn(selection.udn());
				string dd = HttpUtils::httpGet(url);
				cout << dd << endl;

				if (!selection.serviceType().empty()) {
					AutoRef<UPnPService> service = cp.getServiceWithUdnAndServiceType(selection.udn(), selection.serviceType());
					if (!service.nil()) {
						url = cp.getBaseUrlWithUdn(selection.udn()).relativePath(service->getScpdUrl());
						cout << "GET SCPD : " << url.toString() << endl;
						dd = HttpUtils::httpGet(url);
						cout << dd << endl;
					}
				}
			} else {
				cout << " -**- Searching... " << string(buffer) << " **" << endl;
				cp.sendMsearchAndWait(buffer, 3);
				cout << " -**- Searching Done **" << endl;
			}
		} else {
			printList(cp.sessionManager());
		}
	}

	// eventServer.stop();

	cp.stop();

	// server.stop();
    
    return 0;
}
