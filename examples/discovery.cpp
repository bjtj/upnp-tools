#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/StringElement.hpp>
#include <liboslayer/XmlParser.hpp>
#include <libhttp-server/AnotherHttpClient.hpp>
#include <libhttp-server/Url.hpp>
#include <libupnp-tools/SSDPServer.hpp>
#include <libupnp-tools/UPnPModels.hpp>
#include <libupnp-tools/Uuid.hpp>

using namespace std;
using namespace UTIL;
using namespace SSDP;
using namespace HTTP;
using namespace UPNP;
using namespace XML;

int run(int argc, char *args[]);

int main(int argc, char *args[]) {
	return run(argc, args);
}

class DumpResponseHandler : public OnResponseListener {
private:
	string dump;
public:
    DumpResponseHandler() {
    }
    virtual ~DumpResponseHandler() {
    }
    virtual void onTransferDone(HttpResponse & response, DataTransfer * transfer, UTIL::AutoRef<UserData> userData) {
        if (transfer) {
            dump = transfer->getString();
        }
    }
    virtual void onError(OS::Exception & e, UTIL::AutoRef<UserData> userData) {
        cout << "Error/e: " << e.getMessage() << endl;
    }
	string & getDump() {
		return dump;
	}
};

string httpGet(const Url & url) {

	AnotherHttpClient client;
    
    DumpResponseHandler handler;
    client.setOnResponseListener(&handler);
    
    client.setFollowRedirect(true);
    client.setUrl(url);
    client.setRequest("GET", UTIL::LinkedStringMap(), NULL);
    client.execute();

	return handler.getDump();
}

size_t readline(char * buffer, size_t max) {
    if (fgets(buffer, (int)max - 1, stdin)) {
		buffer[strlen(buffer) - 1] = 0;
		return strlen(buffer);
	}
    return 0;
}

class Session {
private:
	static unsigned long idx_seed;
	unsigned long idx;
	string udn;
	string fn;
	string dd;
	bool _completed;
	UPnPDevice device;
	
public:
    Session(const string & udn) : udn(udn), idx(idx_seed++), _completed(false) {}
    virtual ~Session() {printf("[%s] session instance destroyed\n", udn.c_str());}

	unsigned long getUniqueSessionId() {
		return idx;
	}

	NameValue nameValue(XmlNode * node) {
		NameValue nv;
		string name = node->tagName();
		string value = node->getFirstChild()->text();
		nv.getName() = name;
		nv.getValue() = value;
		return nv;
	}

	void buildDevice(SSDPHeader & header) {
		dd = httpGet(header.getLocation());
		device.baseUrl() = header.getLocation();
		XmlDocument doc = DomParser::parse(dd);
		XmlNode * deviceNode = doc.getRootNode()->getElementByTagName("device");
		if (deviceNode) {
			parseDeviceXmlNode(deviceNode);
		}
	}

	void parseDeviceXmlNode(XmlNode * deviceXml) {
		parseDevicePropertiesFromDeviceXmlNode(deviceXml);
		parseServiceListFromDeviceXmlNode(deviceXml);
	}

	void parseDevicePropertiesFromDeviceXmlNode(XmlNode * deviceXml) {
		parsePropertiesFromXmlNode(deviceXml, device);
	}

	void parseServiceListFromDeviceXmlNode(XmlNode * deviceXml) {
		vector<XmlNode*> services = deviceXml->getElementsByTagName("service");
		for (vector<XmlNode*>::iterator iter = services.begin(); iter != services.end(); iter++) {
			AutoRef<UPnPService> service(new UPnPService(NULL));

			parseServicePropertiesFromServiceXmlNode(*iter, &service);

			device.addService(service);
			cout << " === Service Type: " << service->getServiceType() << endl;
			
			buildService(*service);

			for (vector<UPnPAction>::iterator iter = service->actions().begin();
				 iter != service->actions().end(); iter++) {
				
				cout << " - " << (*iter).name() << endl;
			}
		}
	}

	void parseServicePropertiesFromServiceXmlNode(XmlNode * serviceXml, UPnPService * service) {
		parsePropertiesFromXmlNode(serviceXml, *service);
	}

	void buildService(UPnPService & service) {
		if (service.getDevice()) {
			Url u = service.getDevice()->baseUrl().relativePath(service.getScpdUrl());
			string scpd = httpGet(u);
			XmlDocument doc = DomParser::parse(scpd);
			vector<XmlNode*> actions = doc.getRootNode()->getElementsByTagName("action");
			for (vector<XmlNode*>::iterator iter = actions.begin(); iter != actions.end(); iter++) {
				service.addAction(parseActionFromActionXml(*iter));
			}
			vector<XmlNode*> stateVariables = doc.getRootNode()->getElementsByTagName("stateVariable");
		}
	}

	UPnPAction parseActionFromActionXml(XmlNode * actionXml) {
		UPnPAction action;
		XmlNode * name = actionXml->getElementByTagName("name");
		if (testNameValueXmlNode(name)) {
			NameValue nv = nameValue(name);
			action.name() = nv.value();
		}
		vector<XmlNode*> arguments = actionXml->getElementsByTagName("argument");
		for (vector<XmlNode*>::iterator iter = arguments.begin(); iter != arguments.end(); iter++) {
			action.addArgument(parseArgumentFromArgumentXml(*iter));
		}
		return action;
	}

	UPnPArgument parseArgumentFromArgumentXml(XmlNode * argumentXml) {
		UPnPArgument arg;
		vector<XmlNode*> children = argumentXml->children();
		for (vector<XmlNode*>::iterator iter = children.begin(); iter != children.end(); iter++) {
			if (testNameValueXmlNode(*iter)) {
				NameValue nv = nameValue(*iter);
				if (nv.name() == "name") {
					arg.name() = nv.value();
				} else if (nv.name() == "direction") {
					arg.direction() = (nv.value() == "out" ?
									   UPnPArgument::OUT_DIRECTION : UPnPArgument::IN_DIRECTION);
				} else if (nv.name() == "relatedStateVariable") {
					arg.stateVariableName() = nv.value();
				}
			}
		}
		return arg;
	}

	void parsePropertiesFromXmlNode(XmlNode * node, UPnPObject & obj) {
		vector<XmlNode*> children = node->children();
		for (vector<XmlNode*>::iterator iter = children.begin(); iter != children.end(); iter++) {
			if (testNameValueXmlNode(*iter)) {
				NameValue nv = nameValue(*iter);
				if (!nv.value().empty()) {
					obj[nv.name()] = nv.value();
				}
			}
		}
	}

	bool testNameValueXmlNode(XmlNode * node) {
		return node && node->isElement() && node->childrenCount() == 1 &&
			node->getFirstChild() && node->getFirstChild()->isText();
	}

	string getDeviceDescription() {
		return dd;
	}

	string getFriendlyName() {
		return fn;
	}

	bool completed() {
		return _completed;
	}

	UPnPDevice & getDevice() {
		return device;
	}
};

unsigned long Session::idx_seed = 0;

class SessionManager {
private:
	map<string, AutoRef<Session> > sessions;
public:
    SessionManager() {}
    virtual ~SessionManager() {}

	bool has(const string & udn) {
		return (sessions.find(udn) != sessions.end());
	}
	AutoRef<Session> prepareSession(const string & udn) {
		if (!has(udn)) {
			sessions[udn] = AutoRef<Session>(new Session(udn));
		}
		return sessions[udn];
	}
	void remove(const string & udn) {
		sessions.erase(udn);
	}
	size_t size() {
		return sessions.size();
	}
	vector<string> getUdnS() {
		vector<string> ret;
		for (map<string, AutoRef<Session> >::iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
			ret.push_back(iter->first);
		}
		return ret;
	}
	AutoRef<Session> operator[] (const string & udn) {
		return sessions[udn];
	}
};

SessionManager sessionManager;

class MySSDPHandler : public SSDPEventHandler {
private:
public:
    MySSDPHandler() {}
    virtual ~MySSDPHandler() {}
	
	virtual bool filter(SSDPHeader & header) {
		return true;
	}
	virtual void onMsearch(SSDPHeader & header) {
		OS::InetAddress addr = header.getRemoteAddr();
		printf("MSEARCH / ST: %s (%s:%d)\n",
			   header.getSt().c_str(), addr.getHost().c_str(), addr.getPort());
	}
	virtual void onNotify(SSDPHeader & header) {
		OS::InetAddress addr = header.getRemoteAddr();
		if (header.isNotifyAlive()) {
			printf("NOTIFY / alive :: URL: %s (%s:%d)\n",
				   header.getLocation().c_str(), addr.getHost().c_str(), addr.getPort());

			addDevice(header);
			
		} else {
			printf("NOTIFY / byebye :: %s (%s:%d)\n",
				   header.getNt().c_str(), addr.getHost().c_str(), addr.getPort());
		}
	}
	virtual void onMsearchResponse(SSDPHeader & header) {
		OS::InetAddress addr = header.getRemoteAddr();

		Uuid uuid(header.getUsn());
		
		printf("RESP / USN: %s / ST: %s / URL: %s (%s:%d)\n",
			   header.getUsn().c_str(), header.getSt().c_str(),
			   header.getLocation().c_str(), addr.getHost().c_str(), addr.getPort());

		addDevice(header);
	}

	void addDevice(SSDPHeader & header) {
		Uuid uuid(header.getUsn());
		string udn = uuid.getUuid();
		if (!sessionManager.has(udn)) {
			AutoRef<Session> session = sessionManager.prepareSession(udn);
			session->buildDevice(header);
			UPnPDevice & device = session->getDevice();
			cout << " ++ DEVICE : " << device.getUdn() << " - " << device.getFriendlyName() << endl;
		}
	}
};


int run(int argc, char *args[]) {

	SSDPServer ssdp;

	MySSDPHandler handler;
	ssdp.setSSDPEventHandler(&handler);
	ssdp.startAsync(500);

	while (1) {
		char buffer[1024] = {0,};
		if (readline(buffer, sizeof(buffer)) > 0) {
			if (!strcmp(buffer, "q")) {
				break;
			} else {
				cout << " ** Searching..." << endl;
				ssdp.sendMsearchAndGather(buffer, 3);
				cout << " ** Done" << endl;
			}
		} else {
			cout << " == UDN List (" << sessionManager.getUdnS().size() << ") ==" << endl;
			vector<string> lst = sessionManager.getUdnS();
			for (vector<string>::iterator iter = lst.begin(); iter != lst.end(); iter++) {
				AutoRef<Session> session = sessionManager[*iter];
				cout << " > " << session->getDevice().getUdn() << " (" <<
					session->getDevice().getFriendlyName() << ")" << endl;
			}
		}
	}

	ssdp.stop();
    
    return 0;
}
