#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <liboslayer/os.hpp>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/Text.hpp>
#include <liboslayer/XmlParser.hpp>
#include <liboslayer/FileStream.hpp>
#include <liboslayer/Uuid.hpp>
#include <liboslayer/File.hpp>
#include <liboslayer/Logger.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>
#include <libupnp-tools/UPnPActionInvoker.hpp>
#include <libupnp-tools/UPnPActionRequest.hpp>
#include <libupnp-tools/UPnPActionResponse.hpp>
#include <libupnp-tools/NetworkUtil.hpp>
#include <libupnp-tools/UPnPEventSubscriber.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace UPNP;
using namespace HTTP;
using namespace XML;

// #define _DEBUG

static AutoRef<Logger> logger = LoggerFactory::instance().
	getObservingLogger(File::basename(__FILE__));

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

#if defined(_DEBUG)
	LoggerFactory::instance().setProfile("*", "basic", "console");
#endif

	try {
		return run(argc, args);
	} catch (const char * e) {
		cerr << "error occured - " << e << endl;
		return 1;
	} catch (const string & e) {
		cerr << "error occured - " << e << endl;
		return 1;
	} catch (OS::Exception & e) {
		cerr << "error occured - " << e.toString() << endl;
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

/**
 * @brief upnp device printer
 */
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
		str.append(device->friendlyName() + " (" + device->udn().str() + ")");

		vector<AutoRef<UPnPService> > services = device->services();
		for (vector<AutoRef<UPnPService> >::iterator iter = services.begin(); iter != services.end(); iter++) {
			str.append("\n");
			str.append(depth, ' ');
			str.append(" [" + (*iter)->serviceType() + "]");

			vector<UPnPAction> actions = (*iter)->scpd().actions();
			for (vector<UPnPAction>::iterator aiter = actions.begin(); aiter != actions.end(); aiter++) {
				str.append("\n");
				str.append(depth, ' ');
				str.append("  - " + (*aiter).name());
			}
		}
			
		vector<AutoRef<UPnPDevice> > & devices = device->childDevices();
		for (vector<AutoRef<UPnPDevice> >::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			str.append("\n");
			str.append(toString((*iter), depth + 1));
		}
		return str;
	}

	string toBriefString() {
		if (device.nil()) {
			return "(n/a)";
		}
		return toBriefString(device, 0);
	}

	string toBriefString(AutoRef<UPnPDevice> device, int depth) {
		string str;
		str.append(depth, ' ');
		if (depth > 0) { str.append("  - "); }
		str.append(device->friendlyName() + " (" + device->udn().str() + ")");
		vector<AutoRef<UPnPDevice> > & devices = device->childDevices();
		for (vector<AutoRef<UPnPDevice> >::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			str.append("\n");
			str.append(toBriefString((*iter), depth + 1));
		}
		return str;
	}
};

static void printList(const vector<AutoRef<UPnPDevice> > & devices) {
	cout << " == Device List (" << devices.size() << ") ==" << endl;
	size_t i = 0;
	for (vector<AutoRef<UPnPDevice> >::const_iterator iter = devices.begin();
		 iter != devices.end(); iter++, i++) {
		UPnPDevicePrinter printer(*iter);
		cout << "[" << i << "] " << printer.toBriefString() << endl;
	}
}

AutoRef<UPnPDevice> selectDeviceByIndex(const vector<AutoRef<UPnPDevice> > & devices, size_t idx) {

	if (idx >= devices.size()) {
		return AutoRef<UPnPDevice>();
	}

	UPnPDevicePrinter printer(devices[idx]);
	cout << printer.toString() << endl;

	return devices[idx];
}

/**
 * @brief device listener
 */
class MyDeviceListener : public UPnPDeviceListener {
private:
public:
    MyDeviceListener() {}
    virtual ~MyDeviceListener() {}

	virtual void onDeviceAdded(AutoRef<UPnPDevice> device) {
		cout << " ** Added: " << device->friendlyName() << endl;
	}

	virtual void onDeviceRemoved(AutoRef<UPnPDevice> device) {
		cout << " ** Removed: " << device->friendlyName() << endl;
	}
};

/**
 * @brief session
 */
class Session {
private:
	UDN _udn;
	string _serviceType;
	string _action;
public:
    Session() {}
    virtual ~Session() {}
	UDN & udn() {return _udn;}
	string & serviceType() {return _serviceType;}
	string & action() {return _action;}
};

static string s_str(const string & s, const string & e) {
	return (s.empty() ? e : s);
}

static void printSession(Session & session) {
	cout << " -- Session --" << endl;
	cout << " |UDN: " << s_str(session.udn().toString(), "(none)") << endl;
	cout << " |Service: " << s_str(session.serviceType(), "(none)") << endl;
	cout << " |Action: " << s_str(session.action(), "(none)") << endl;
}

/**
 * @brief print debug info
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

/**
 * @brief run
 */
int run(int argc, char *args[]) {

	FileStream out("./.controller.log", "wb");
	Session session;

	AutoRef<UPnPDebug> debug(new UPnPDebug);
	debug->setOnDebugInfoListener(AutoRef<OnDebugInfoListener>(new PrintDebugInfo(out)));
	AutoRef<SharedUPnPDeviceList> list(new SharedUPnPDeviceList);
	UPnPControlPoint cp(UPnPControlPoint::Config(9998));
	cp.setDebug(debug);
	cp.addSharedDeviceList(list);
	cp.setDeviceListener(AutoRef<UPnPDeviceListener>(new MyDeviceListener));
	cp.startAsync();
	cp.getEventReceiver()->addEventListener(AutoRef<UPnPEventListener>(new MyEventListener));

	while (1) {
		string line;
		if ((line = readline()).size() == 0) {
			printList(cp.getDevices());
			cout << endl;
			printSession(session);
			continue;
		}		
		if (line == "q" || line == "quit") {
			cout << "[quit]" << endl;
			break;
		} else if (line == "clear") {
			cp.clearDevices();
		} else if (line.find_first_not_of("0123456789") == string::npos) {
			int idx = Text::toInt(line);
			cout << "idx : " << idx << endl;
			AutoRef<UPnPDevice> device = selectDeviceByIndex(cp.getDevices(), (size_t)idx);
			if (device.nil() == false) {
				session.udn() = device->udn();
				cout << "* SET UDN> " << session.udn().toString() << endl;
			}
		} else if (Text::startsWith(line, "udn ")) {
			session.udn() = UDN(line.substr(4));
		} else if (line == "udn") {
			cout << "[UDN: " << session.udn().toString() << "]" << endl;
		} else if (Text::startsWith(line, "service ")) {
			session.serviceType() = line.substr(8);
		} else if (line == "service") {
			cout << "[Service : " << session.serviceType() << "]" << endl;
		} else if (Text::startsWith(line, "action ")) {
			session.action() = line.substr(7);
		} else if (line == "action") {
			cout << "[Action : " << session.action() << "]" << endl;
		} else if (line == "invoke" || line == "i") {
			try {
				UPnPActionInvoker invoker = cp.prepareActionInvoke(session.udn(), session.serviceType());
				AutoRef<UPnPService> service = cp.getServiceByUdnAndServiceType(session.udn(), session.serviceType());
					
				if (!service->scpd().hasAction(session.action())) {
					throw "[Error: no action found]";
				}
				UPnPAction action = service->scpd().action(session.action());
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
				unsigned long tick = tick_milli();
				UPnPActionResponse response = invoker.invoke(request);
				LinkedStringMap & params = response.parameters();
				for (size_t i = 0; i < params.size(); i++) {
					KeyValue & kv = params[i];
					string name = kv.key();
					string & value = kv.value();
					cout << " - " << name << " := " << value << endl;
				}
				cout << "[elapsed : " << tick_milli() - tick << " ms.]" << endl;
			} catch (OS::Exception & e) {
				cout << "[error : " << e.toString() << "]" << endl;
			}
		} else if (line == "subs") {
			// TODO: subscription list
		} else if (line == "sub") {
			if (session.udn().empty() || session.serviceType().empty()) {
				throw "[error: select udn and sevice first]";
			}
			cout << "[Subscribe - " << session.udn().toString() << " // " << session.serviceType() << "]" << endl;
			cp.subscribe(session.udn(), session.serviceType());
		} else if (line == "unsub") {
			if (session.udn().empty() || session.serviceType().empty()) {
				throw "[error: select udn and sevice first]";
			}
			cout << "[Unsubscribe - " << session.udn().toString() << " .. " << session.serviceType() << "]" <<endl;
			cp.unsubscribe(session.udn(), session.serviceType());
		} else if (line == "shared") {
			vector<AutoRef<UPnPDevice> > devices;
			list->list(devices);
			for (vector< AutoRef<UPnPDevice> >::iterator iter = devices.begin(); iter != devices.end(); iter++) {
				cout << " * " << (*iter)->friendlyName() << endl;
			}
		} else if (line == "dump") {
			if (session.udn().empty()) {
				throw "[error: select udn first]";
			}
			Url url = cp.getBaseUrlByUdn(session.udn());
			string dd = HttpUtils::httpGet(url);
			cout << dd << endl;
			if (!session.serviceType().empty()) {
				AutoRef<UPnPService> service = cp.getServiceByUdnAndServiceType(session.udn(), session.serviceType());
				if (!service.nil()) {
					url = cp.getBaseUrlByUdn(session.udn()).relativePath(service->scpdUrl());
					cout << "[GET SCPD : " << url.toString() << "]" << endl;
					dd = HttpUtils::httpGet(url);
					cout << dd << endl;
				}
			}
		} else {
			cout << "[Searching : '" << line << "']" << endl;
			cp.sendMsearchAsync(line, 3);
		}
	}
	cp.stop();
	out.close();
	cout << "[bye]" << endl;
    return 0;
}
