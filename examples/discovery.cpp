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
#include <libupnp-tools/UPnPSession.hpp>
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



UPnPSessionManager sessionManager;

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
		printf("> MSEARCH / ST: %s (%s:%d)\n",
			   header.getSt().c_str(), addr.getHost().c_str(), addr.getPort());
	}
	virtual void onNotify(SSDPHeader & header) {
		OS::InetAddress addr = header.getRemoteAddr();
		if (header.isNotifyAlive()) {
			printf("> NOTIFY / alive :: URL: %s (%s:%d)\n",
				   header.getLocation().c_str(), addr.getHost().c_str(), addr.getPort());

			addDevice(header);
			
		} else {
			printf("> NOTIFY / byebye :: %s (%s:%d)\n",
				   header.getNt().c_str(), addr.getHost().c_str(), addr.getPort());

			removeDevice(header);
		}
	}
	virtual void onMsearchResponse(SSDPHeader & header) {
		OS::InetAddress addr = header.getRemoteAddr();

		Uuid uuid(header.getUsn());
		
		printf("> RESP / USN: %s / ST: %s / URL: %s (%s:%d)\n",
			   header.getUsn().c_str(), header.getSt().c_str(),
			   header.getLocation().c_str(), addr.getHost().c_str(), addr.getPort());

		addDevice(header);
	}

	void addDevice(SSDPHeader & header) {
		Uuid uuid(header.getUsn());
		string udn = uuid.getUuid();
		if (!sessionManager.has(udn)) {
			AutoRef<UPnPSession> session = sessionManager.prepareSession(udn);
			session->buildDevice(header);
			UPnPDevice & device = session->getDevice();
			cout << " ++ DEVICE : " << device.getUdn() << " - " << device.getFriendlyName() << endl;
		}
	}

	void removeDevice(SSDPHeader & header) {
		Uuid uuid(header.getUsn());
		string udn = uuid.getUuid();
		sessionManager.remove(udn);
		cout << " -- DEVICE : " << udn << endl;
	}
};

void printList() {
	cout << " == Device List (" << sessionManager.getUdnS().size() << ") ==" << endl;
	vector<string> lst = sessionManager.getUdnS();
	size_t i = 0;
	for (vector<string>::iterator iter = lst.begin(); iter != lst.end(); iter++, i++) {
		AutoRef<UPnPSession> session = sessionManager[*iter];
		cout << " [" << i << "] " << session->getDevice().getUdn() << " (" <<
			session->getDevice().getFriendlyName() << ")" << endl;
	}
}

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
			} else if (!strcmp(buffer, "clear")) {
				sessionManager.clear();
			} else {
				cout << " ** Searching..." << endl;
				ssdp.sendMsearchAndGather(buffer, 3);
				cout << " ** Done" << endl;
			}
		} else {
			printList();
		}
	}

	ssdp.stop();
    
    return 0;
}
