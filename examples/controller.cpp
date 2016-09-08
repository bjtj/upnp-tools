#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/Text.hpp>
#include <liboslayer/XmlParser.hpp>
#include <liboslayer/FileStream.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>
#include <libupnp-tools/UPnPActionInvoker.hpp>
#include <libupnp-tools/UPnPActionRequest.hpp>
#include <libupnp-tools/UPnPActionResponse.hpp>
#include <libupnp-tools/NetworkUtil.hpp>
#include <libupnp-tools/UPnPEventSubscriber.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>

using namespace std;
using namespace UTIL;
using namespace UPNP;
using namespace HTTP;
using namespace XML;

class MyEventListener : public UPnPEventListener {
private:
public:
    MyEventListener() {}
    virtual ~MyEventListener() {}
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

string readline() {
	FileStream fs(stdin);
	return fs.readline();
}

string prompt(const string & msg) {
	cout << msg;
	return readline();
}

class UPnPDevicePrinter {
private:
	AutoRef<UPnPDevice> device;
public:
    UPnPDevicePrinter(AutoRef<UPnPDevice> device) : device(device) {}
    virtual ~UPnPDevicePrinter() {}
	
	string toString() {
		if (device.nil()) {
			return "(null)";
		}
		return toString(device, 0);
	}

	string toString(AutoRef<UPnPDevice> device, int depth) {
		string str;

		str.append(depth, ' ');
		if (depth > 0) { str.append(" - "); }
		str.append(device->getUdn() + " (" + device->getFriendlyName() + ")");

		vector<AutoRef<UPnPService> > services = device->services();
		for (vector<AutoRef<UPnPService> >::iterator iter = services.begin(); iter != services.end(); iter++) {
			str.append("\n");
			str.append(depth, ' ');
			str.append(" <> " + (*iter)->serviceType());

			vector<UPnPAction> actions = (*iter)->scpd().actions();
			for (vector<UPnPAction>::iterator aiter = actions.begin(); aiter != actions.end(); aiter++) {
				str.append("\n");
				str.append(depth, ' ');
				str.append("  - " + (*aiter).name());
			}
		}
			
		vector<AutoRef<UPnPDevice> > & devices = device->embeddedDevices();
		for (vector<AutoRef<UPnPDevice> >::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			str.append("\n");
			str.append(toString((*iter), depth + 1));
		}
		return str;
	}

	string toBriefString() {
		if (device.nil()) {
			return "";
		}
		return toBriefString(device, 0);
	}

	string toBriefString(AutoRef<UPnPDevice> device, int depth) {
		string str;

		str.append(depth, ' ');
		if (depth > 0) { str.append("  - "); }
		str.append(device->getUdn() + " (" + device->getFriendlyName() + ")");

		vector<AutoRef<UPnPDevice> > & devices = device->embeddedDevices();
		for (vector<AutoRef<UPnPDevice> >::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			str.append("\n");
			str.append(toBriefString((*iter), depth + 1));
		}
		return str;
	}
};

void printList(const vector<AutoRef<UPnPDevice> > & devices) {

	cout << " == Device List (" << devices.size() << ") ==" << endl;
	
	size_t i = 0;
	for (vector<AutoRef<UPnPDevice> >::const_iterator iter = devices.begin(); iter != devices.end(); iter++, i++) {
		UPnPDevicePrinter printer(*iter);
		cout << "[" << i << "] " << printer.toBriefString() << endl;
	}
}

void selectDeviceByIndex(const vector<AutoRef<UPnPDevice> > & devices, size_t idx) {

	if (idx >= devices.size()) {
		return;
	}

	UPnPDevicePrinter printer(devices[idx]);
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

/**
 * @brief 
 */
class PrintDebugInfo : public OnDebugInfoListener {
private:
	FileStream & stream;
public:
	PrintDebugInfo(FileStream & stream) : stream(stream) {
	}
	virtual ~PrintDebugInfo() {
	}
	virtual void onDebugInfo(const UPnPDebugInfo & info) {
		stream.writeline(info.const_packet());
	}
};

int run(int argc, char *args[]) {

	FileStream out("./.controller.log", "wb");

	AutoRef<UPnPDebug> debug(new UPnPDebug);
	debug->setOnDebugInfoListener(AutoRef<OnDebugInfoListener>(new PrintDebugInfo(out)));

	AutoRef<SharedUPnPDeviceList> list(new SharedUPnPDeviceList);

	Selection selection;

	UPnPControlPointConfig config(9998);
	UPnPControlPoint cp(config);
	cp.setDebug(debug);

	cp.addSharedDeviceList(list);
	cp.setDeviceAddRemoveListener(AutoRef<DeviceAddRemoveListener>(new MyDeviceListener));
	cp.startAsync();

	cp.getEventReceiver()->addEventListener(AutoRef<UPnPEventListener>(new MyEventListener));

	while (1) {
		string line;
		if ((line = readline()).size() > 0) {
			if (line == "q") {
				break;
			} else if (line == "clear") {
				cp.clearDevices();
			} else if (line.find_first_not_of("0123456789") == string::npos) {
				int idx = Text::toInt(line);
				cout << "idx : " << idx << endl;
				selectDeviceByIndex(cp.getDevices(), (size_t)idx);
			} else if (Text::startsWith(line, "udn ")) {
				selection.udn() = line.substr(4);
			} else if (line == "udn") {
				cout << "UDN: " << selection.udn() << endl;
			} else if (Text::startsWith(line, "service ")) {
				selection.serviceType() = line.substr(8);
			} else if (line == "service") {
				cout << "Service : " << selection.serviceType() << endl;
			} else if (Text::startsWith(line, "action ")) {
				selection.action() = line.substr(7);
			} else if (line == "action") {
				cout << "Action : " << selection.action() << endl;
			} else if (line == "invoke") {
				try {
					UPnPActionInvoker invoker = cp.prepareActionInvoke(selection.udn(), selection.serviceType());
					AutoRef<UPnPService> service = cp.getServiceByUdnAndServiceType(selection.udn(), selection.serviceType());
					
					if (!service->scpd().hasAction(selection.action())) {
						throw "Error: no action found";
					}
					UPnPAction action = service->scpd().action(selection.action());
					vector<UPnPArgument> & arguments = action.arguments();
					UPnPActionRequest request;
					request.serviceType() = service->serviceType();
					request.action() = action;
					for (vector<UPnPArgument>::iterator iter = arguments.begin(); iter != arguments.end(); iter++) {
						if (iter->direction() == UPnPArgument::IN_DIRECTION) {
							string allows = "";
							UPnPStateVariable sv = service->scpd().stateVariable(iter->relatedStateVariable());
							if (sv.hasAllowedValues()) {
								allows = " [" + Text::join(sv.allowedValueList(), ", ") + "]";
							}
							string param = prompt(iter->name() + allows + " : ");
							request[iter->name()] = param;
						}
					}
					UPnPActionResponse response = invoker.invoke(request);
					map<string, string> & params = response.parameters();
					for (map<string, string>::iterator iter = params.begin(); iter != params.end(); iter++) {
						string name = iter->first;
						string & value = iter->second;

						cout << " - " << name << " := " << value << endl;
					}
				} catch (OS::Exception & e) {
					cout << "Error: " << e.getMessage() << endl;
				}
			} else if (line == "subs") {
				// TODO: subscription list
			} else if (line == "sub") {

				if (selection.udn().empty() || selection.serviceType().empty()) {
					throw "Error: select udn and sevice first";
				}

				cout << "Subscribe - " << selection.udn() << " // " << selection.serviceType() << endl;
				cp.subscribe(selection.udn(), selection.serviceType());

			} else if (line == "unsub") {

				if (selection.udn().empty() || selection.serviceType().empty()) {
					throw "Error: select udn and sevice first";
				}

				cout << "Unsubscribe - " << selection.udn() << " .. " << selection.serviceType() << endl;
				cp.unsubscribe(selection.udn(), selection.serviceType());

			} else if (line == "shared") {

				vector<AutoRef<UPnPDevice> > devices = list->list_s();

				for (vector<AutoRef<UPnPDevice> >::iterator iter = devices.begin(); iter != devices.end(); iter++) {
					cout << " * " << (*iter)->getFriendlyName() << endl;
				}

			} else if (line == "dump") {

				if (selection.udn().empty()) {
					throw "Error: select udn first";
				}

				Url url = cp.getBaseUrlByUdn(selection.udn());
				string dd = HttpUtils::httpGet(url);
				cout << dd << endl;

				if (!selection.serviceType().empty()) {
					AutoRef<UPnPService> service = cp.getServiceByUdnAndServiceType(selection.udn(), selection.serviceType());
					if (!service.nil()) {
						url = cp.getBaseUrlByUdn(selection.udn()).relativePath(service->scpdUrl());
						cout << "GET SCPD : " << url.toString() << endl;
						dd = HttpUtils::httpGet(url);
						cout << dd << endl;
					}
				}
			} else {
				cout << " -**- Searching... " << line << " **" << endl;
				cp.sendMsearchAsync(line, 3);
			}
		} else {
			printList(cp.getDevices());
		}
	}

	cp.stop();

    return 0;
}
