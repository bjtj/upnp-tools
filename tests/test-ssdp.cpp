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

class TestSSDPHandler : public SSDPEventHandler {
private:
	vector<SSDPHeader> _headers;
public:
    TestSSDPHandler() {}
    virtual ~TestSSDPHandler() {}

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
};


static void test_discovery() {

	TestSSDPHandler * tsh = new TestSSDPHandler;
	AutoRef<SSDPEventHandler> handler(tsh);

	SSDPServer server;
	server.setSSDPEventHandler(handler);
	server.startAsync(100);

	idle(1000);

	mcast("NOTIFY * HTTP/1.1\r\n\r\n");

	idle(1000);

	server.stop();

	ASSERT(tsh->headers()[0].getRawPacket(), ==, "NOTIFY * HTTP/1.1\r\n\r\n");
}

int main(int argc, char *args[]) {

	test_discovery();
    
    return 0;
}
