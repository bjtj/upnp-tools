#include <iostream>
#include <libupnp-tools/SSDPServer.hpp>
#include "utils.hpp"

using namespace std;
using namespace SSDP;
using namespace OS;
using namespace UTIL;

#define SSDP_HOST "239.255.255.250"
#define SSDP_PORT 1900

static void mcast(const string & packet) {
	SSDPMsearchSender sender;
	sender.sendMcastToAllInterfaces(packet, SSDP_HOST, SSDP_PORT);
	sender.close();
}

class TestSSDPListener : public SSDPEventListener {
private:
	vector<SSDPHeader> _headers;
public:
    TestSSDPListener() {}
    virtual ~TestSSDPListener() {}

	virtual bool filter(SSDPHeader & header) {
		return true;
	}
	virtual void onMsearch(SSDPHeader & header) {
	}
	virtual void onNotify(SSDPHeader & header) {
		_headers.push_back(header);
	}
	virtual void onMsearchResponse(SSDPHeader & header) {
	}
	vector<SSDPHeader> & headers() {
		return _headers;
	}
	SSDPHeader & findHeaderWithRawPacket(const string & rawPacket) {
		for (vector<SSDPHeader>::iterator iter = _headers.begin(); iter != _headers.end(); iter++) {
			if (iter->getRawPacket() == rawPacket) {
				return *iter;
			}
		}
		throw Exception("not found");
	}
	bool hasHeaderWithRawPacket(const string & rawPacket) {
		for (vector<SSDPHeader>::iterator iter = _headers.begin(); iter != _headers.end(); iter++) {
			if (iter->getRawPacket() == rawPacket) {
				return true;
			}
		}
		return false;
	}
};

static void test_discovery() {

	TestSSDPListener * pListener = new TestSSDPListener;
	AutoRef<SSDPEventListener> listener(pListener);

	SSDPServer server;
	server.addSSDPEventListener(listener);
	server.startAsync();

	idle(1000);

	mcast("NOTIFY * HTTP/1.1\r\n\r\n");

	idle(1000);

	server.stop();

	ASSERT(pListener->hasHeaderWithRawPacket("NOTIFY * HTTP/1.1\r\n\r\n"), ==, true);
}

int main(int argc, char *args[]) {

	test_discovery();
    
    return 0;
}
