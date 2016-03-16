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
#include <libhttp-server/AnotherHttpServer.hpp>

using namespace std;
using namespace UTIL;
using namespace UPNP;
using namespace HTTP;
using namespace XML;

class Subscription {
private:
	string _sid;
	unsigned long _lastSeq;
	string _udn;
	string _serviceType;
	
public:
    Subscription() {}
    virtual ~Subscription() {}
	string & sid() {return _sid;}
	unsigned long & lastSeq() {return _lastSeq;}
	string & udn() {return _udn;}
	string & serviceType() {return _serviceType;}
};

class SubscriptionRegstry {
private:
	map<string, Subscription> subs;
public:
    SubscriptionRegstry() {}
    virtual ~SubscriptionRegstry() {}

	map<string, Subscription> & subscriptions() {return subs;}
	bool has(const string & sid) {return subs.find(sid) != subs.end();}
	void add(Subscription & sub) {subs[sub.sid()] = sub;}
	void del(const string & sid) {subs.erase(sid);}
	void del(Subscription sub) {subs.erase(sub.sid());}
	bool hasWithUdnAndServiceType(const string & udn, const string & serviceType) {
		for (map<string, Subscription>::iterator iter = subs.begin(); iter != subs.end(); iter++) {
			if (iter->second.udn() == udn && iter->second.serviceType() == serviceType) {
				return true;
			}
		}
		return false;
	}
	Subscription & findWithUdnAndServiceType(const string & udn, const string & serviceType) {
		for (map<string, Subscription>::iterator iter = subs.begin(); iter != subs.end(); iter++) {
			if (iter->second.udn() == udn && iter->second.serviceType() == serviceType) {
				return iter->second;
			}
		}
		throw OS::Exception("not found subscription", -1, 0);
	}
	Subscription & operator[] (const string & sid) {
		return subs[sid];
	}
};

class EventNotificationParser {
private:
public:
    EventNotificationParser() {}
    virtual ~EventNotificationParser() {}

	static map<string, string> parseNotify(const string & xml) {
		map<string, string> props;
		XML::XmlDocument doc = XML::DomParser::parse(xml);
		if (doc.getRootNode().nil()) {
			return props;
		}
		vector<XmlNode*> nodes = doc.getRootNode()->getElementsByTagName("property");
		for (vector<XmlNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++) {
			XmlNode * node = *iter;
			NameValue nv = XmlUtils::toNameValue(node->getFirstChildElement());
			props[nv.name()] = nv.value();
		}
		return props;
	}
};

class EventNotificationHandler : public HttpRequestHandler {
private:
public:
    EventNotificationHandler() {}
    virtual ~EventNotificationHandler() {}

	virtual void onHttpRequestContentCompleted(HttpRequest & request, HttpResponse & response) {
		AutoRef<DataTransfer> transfer = request.getTransfer();
		if (!transfer.nil()) {
			string sid = request.getHeader()["SID"];
			string seq = request.getHeader()["SEQ"];
			cout << " >>> Notify <<< " << endl;
			cout << " ** SID : " << sid << endl;
			cout << " ** SEQ : " << seq << endl;
			map<string, string> props = EventNotificationParser::parseNotify(transfer->getString());
			for (map<string, string>::iterator iter = props.begin(); iter != props.end(); iter++) {
				cout << "  -- " << iter->first << " : " << iter->second << endl;
			}
		}
	}	
};

Subscription sendSubscribe(Url & url, vector<string> callbackUrls) {
	Subscription sub;
	UTIL::LinkedStringMap headers;
	string callbackUrlStrings;
	for (vector<string>::iterator iter = callbackUrls.begin(); iter != callbackUrls.end(); iter++) {
		if (callbackUrlStrings.size() > 0) {
			callbackUrlStrings.append(" ");
		}
		callbackUrlStrings.append("<" + *iter + ">");
	}
	headers["CALLBACK"] = callbackUrlStrings;
	headers["NT"] = "upnp:event";
	headers["TIMEOUT"] = "Second-300";
	HttpResponseHeader header = HttpUtils::dumpHttpRequest(url, "SUBSCRIBE", headers).getResponseHeader();
	string sid = header["SID"];
	sub.sid() = sid;
	return sub;
}

void sendUnsubscribe(Url & url, const string & sid) {
	UTIL::LinkedStringMap headers;
	headers["SID"] = sid;
	int ret = HttpUtils::dumpHttpRequest(url, "UNSUBSCRIBE", headers).getResponseHeader().getStatusCode();
}

static int run(int argc, char *args[]);

int main(int argc, char *args[]) {

	int ret = 0;
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

	SubscriptionRegstry subs;

	HttpServerConfig config;
	config["listen.port"] = "9998";
	config["thread.count"] = "5";
	AnotherHttpServer server(config);
	UPnPControlPoint cp;

	AutoRef<HttpRequestHandler> handler(new EventNotificationHandler);
	server.registerRequestHandler("/*", handler);
	server.startAsync();

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
							char param[1024] = {0,};
							prompt(iter->name() + " : ", param, sizeof(param));
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

				map<string, Subscription> & lst = subs.subscriptions();
				cout << " == Subscriptions (" << lst.size() << ") == " << endl;
				for (map<string, Subscription>::iterator iter = lst.begin(); iter != lst.end(); iter++) {
					Subscription sub = iter->second;
					cout << "  ** SID: " << sub.sid() << " (" << sub.udn() << " / " << sub.serviceType() << ")" << endl;
				}
				
			} else if (!strcmp(buffer, "sub")) {

				if (selection.udn().empty() || selection.serviceType().empty()) {
					throw "Error: select udn and sevice first";
				}

				AutoRef<UPnPService> service = cp.getServiceWithUdnAndServiceType(selection.udn(), selection.serviceType());
				Url url = cp.getBaseUrlWithUdn(selection.udn()).relativePath(service->getEventSubUrl());
				vector<string> callbackUrls;
				OS::InetAddress addr = NetworkUtil::selectDefaultAddress();
				callbackUrls.push_back("http://" + addr.getHost() + ":9998/notify");
				Subscription sub = sendSubscribe(url, callbackUrls);
				sub.udn() = selection.udn();
				sub.serviceType() = selection.serviceType();
				subs.add(sub);
				
			} else if (!strcmp(buffer, "unsub")) {

				if (selection.udn().empty() || selection.serviceType().empty()) {
					throw "Error: select udn and sevice first";
				}

				AutoRef<UPnPService> service = cp.getServiceWithUdnAndServiceType(selection.udn(), selection.serviceType());
				Url url = cp.getBaseUrlWithUdn(selection.udn()).relativePath(service->getEventSubUrl());

				if (subs.hasWithUdnAndServiceType(selection.udn(), selection.serviceType())) {
					Subscription sub = subs.findWithUdnAndServiceType(selection.udn(), selection.serviceType());
					sendUnsubscribe(url, sub.sid());
					subs.del(sub);
				}
				
			} else {
				cout << " ** Searching... " << string(buffer) << " **" << endl;
				cp.sendMsearchAndWait(buffer, 3);
				cout << endl << " ** Searching Done **" << endl;
			}
		} else {
			printList(cp.sessionManager());
		}
	}

	cp.stop();

	server.stop();
    
    return 0;
}
