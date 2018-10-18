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
#include <libhttp-server/AnotherHttpServer.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>
#include <libupnp-tools/UPnPActionInvoker.hpp>
#include <libupnp-tools/UPnPActionRequest.hpp>
#include <libupnp-tools/UPnPActionResponse.hpp>
#include <libupnp-tools/NetworkUtil.hpp>
#include <libupnp-tools/UPnPEventSubscriber.hpp>
#include <libupnp-tools/HttpUtils.hpp>


using namespace std;
using namespace osl;
using namespace upnp;
using namespace http;


static AutoRef<Logger> logger = LoggerFactory::instance().
	getObservingLogger(File::basename(__FILE__));

class MyEventListener : public UPnPEventListener {
private:
public:
    MyEventListener() {}
    virtual ~MyEventListener() {}
	virtual void onNotify(UPnPPropertySet & propset) {
		cout << "Notify : " << propset.sid() << endl;
		vector<string> names = propset.propertyNames();
		for (vector<string>::iterator iter = names.begin(); iter != names.end(); iter++) {
			string value = propset[*iter];
			cout << " - " << *iter << " : " << value << endl;
		}
	}
};


static int run(int argc, char *args[]);


/**
 * 
 */
class FileWriter : public LogWriter {
private:
	FileStream _stream;
public:
	FileWriter(const string & path) : _stream(path, "wb") {
	}
	
	virtual ~FileWriter() {
	}
	
	virtual void write(const string & str) {
		_stream.writeline(str);
	}
};


int main(int argc, char *args[]) {

	AutoRef<LogWriter> writer(new FileWriter(".controlpoint.log"));
	LoggerFactory::instance().registerWriter("filewriter", writer);
	LoggerFactory::instance().setProfile("UPnP*", "basic", "filewriter");

	try {
		return run(argc, args);
	} catch (const char * e) {
		cerr << "error occured - " << e << endl;
		return 1;
	} catch (const string & e) {
		cerr << "error occured - " << e << endl;
		return 1;
	} catch (Exception & e) {
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
		str.append(device->friendlyName() + " (" + device->udn() + ")");

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
		str.append(device->friendlyName() + " (" + device->udn() + ")");
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
	string _udn;
	string _serviceType;
	string _action;
public:
    Session() {}
    virtual ~Session() {}
	string & udn() {return _udn;}
	string & serviceType() {return _serviceType;}
	string & action() {return _action;}
};

static string s_str(const string & s, const string & e) {
	return (s.empty() ? e : s);
}

static void printSession(Session & session) {
	cout << " -- Selection --" << endl;
	cout << " | UDN: " << s_str(session.udn(), "(none)") << endl;
	cout << " | Service: " << s_str(session.serviceType(), "(none)") << endl;
	cout << " | Action: " << s_str(session.action(), "(none)") << endl;
}


/**
 * @brief run
 */
int run(int argc, char *args[]) {

	Session session;
	AutoRef<SharedUPnPDeviceList> list(new SharedUPnPDeviceList);
	UPnPControlPoint cp(UPnPControlPoint::Config(9998));
	cp.addSharedDeviceList(list);
	cp.setDeviceListener(AutoRef<UPnPDeviceListener>(new MyDeviceListener));
	cp.startAsync();
	cp.getEventReceiver()->addEventListener(AutoRef<UPnPEventListener>(new MyEventListener));

	cout << "h|help -- display help" << endl;

	while (1) {
		string line;
		if ((line = readline()).size() == 0) {
			printList(cp.getDevices());
			cout << endl;
			printSession(session);
			continue;
		}
		
		if (line == "h" || line == "help") {
			cout << endl;
			cout << "HELP" << endl;
			cout << "====" << endl;
			cout << endl;
			cout << "  h|help -- display help (this)" << endl;
			cout << "  q|quit -- quit" << endl;
			cout << "  all -- send msearch (ssdp:all)" << endl;
			cout << "  search <st> -- send msearch (<st>)" << endl;
			cout << "  clear -- remove all devices" << endl;
			cout << "  [0-9]+ -- select device " << endl;
			cout << "  udn <udn> -- select <udn>" << endl;
			cout << "  service <service> -- select <service>" << endl;
			cout << "  action <action> -- select <action>" << endl;
			cout << "  i|invoke -- invoke action (required selected service and action)" << endl;
			cout << "  subscriptions -- list subscriptions" << endl;
			cout << "  subscribe -- subscribe event (required selected service)" << endl;
			cout << "  unsubscribe -- unsubscribe event (required selected service)" << endl;
			cout << "  (empty) -- print device list and selection status" << endl;
			cout << endl;
		} else if (line == "q" || line == "quit") {
			cout << "[quit]" << endl;
			break;
		} else if (line == "all") {
			string st = "ssdp:all";
			cout << "[search all] type: '" << st << "'" << endl;
			cp.sendMsearchAsync(st, 3);
		} else if (Text::startsWith(line, "search ")) {
			string st = line.substr(string("search ").size());
			cout << "[search] type: '" << st << "'" << endl;
			cp.sendMsearchAsync(st, 3);
		} else if (line == "clear") {
			cp.clearDevices();
		} else if (line.find_first_not_of("0123456789") == string::npos) {
			int idx = Text::toInt(line);
			cout << "idx : " << idx << endl;
			AutoRef<UPnPDevice> device = selectDeviceByIndex(cp.getDevices(), (size_t)idx);
			if (device.nil()) {
				cout << "No device" << endl;
			} else {
				session.udn() = device->udn();
				cout << "Select device -- udn: " << session.udn() << endl;
			}
		} else if (Text::startsWith(line, "udn ")) {
			session.udn() = line.substr(4);
		} else if (line == "udn") {
			cout << "[session] udn: " << session.udn() << endl;
		} else if (Text::startsWith(line, "service ")) {
			session.serviceType() = line.substr(8);
		} else if (line == "service") {
			cout << "[session] service: " << session.serviceType() << endl;
		} else if (Text::startsWith(line, "action ")) {
			session.action() = line.substr(7);
		} else if (line == "action") {
			cout << "[session] action: " << session.action() << endl;
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
				cout << "[Invoke] " << tick_milli() - tick << " ms." << endl;
			} catch (Exception & e) {
				cout << "[Error] " << e.toString() << endl;
			}
		} else if (line == "subscriptions") {
			map< string, UPnPEventSubscription > subscriptions = cp.getSubscriptions();
			cout << " == Subscriptions ==" << endl;
			cout << "count: " << subscriptions.size() << endl;
			for (map< string, UPnPEventSubscription >::iterator iter = subscriptions.begin();
				 iter != subscriptions.end(); iter++)
			{
				cout << " * " << iter->first << endl;
			}
		} else if (line == "subscribe") {
			if (session.udn().empty() || session.serviceType().empty()) {
				throw "[error] select udn and sevice first";
			}
			cout << "[Subscribe] udn: " << session.udn() << " / service: " << session.serviceType() << endl;
			cp.subscribe(session.udn(), session.serviceType());
		} else if (line == "unsubscribe") {
			if (session.udn().empty() || session.serviceType().empty()) {
				throw "[error] select udn and sevice first";
			}
			cout << "[Unsubscribe] udn: " << session.udn() << " / service: " << session.serviceType() << endl;
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
					cout << "[dump] get scpd : " << url.toString() << endl;
					dd = HttpUtils::httpGet(url);
					cout << dd << endl;
				}
			}
		} else {
			cout << "Unknown command -- " << line << endl;
		}
	}
	cp.stop();
	cout << "[bye]" << endl;
    return 0;
}
