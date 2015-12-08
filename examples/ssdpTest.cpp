#include <liboslayer/os.hpp>
#include <liboslayer/DatagramSocket.hpp>
#include <liboslayer/Text.hpp>
#include <libhttp-server/HttpHeader.hpp>
#include <libhttp-server/HttpHeaderReader.hpp>
#include <libupnp-tools/SSDPHeader.hpp>
#include <libupnp-tools/SSDPPacketHandler.hpp>
#include <libupnp-tools/SSDPMulticastListener.hpp>

using namespace OS;
using namespace HTTP;
using namespace UTIL;
using namespace SSDP;

class MySSDPPacketListener : public SSDPPacketHandler {
private:
public:
	MySSDPPacketListener() {}
	virtual ~MySSDPPacketListener() {}
	virtual void onAnyPacket(SSDPHeader & header) {
		// printf("FROM %s:%d\n%s", header.getRemoteAddr().getHost().c_str(), header.getRemoteAddr().getPort(), header.toString().c_str());
	}
	virtual void onMsearch(SSDPHeader & header) {
		printf("onMsearch :: %s (%s:%d)\n", header.getSt().c_str(), header.getRemoteAddr().getHost().c_str(), header.getRemoteAddr().getPort());
	}
	virtual void onNotify(SSDPHeader & header) {
		printf("onNotify :: %s (%s:%d)\n", header.getLocation().c_str(), header.getRemoteAddr().getHost().c_str(), header.getRemoteAddr().getPort());
	}
	virtual void onResponse(SSDPHeader & header) {
		printf("onResponse :: %s (%s:%d)\n", header.getLocation().c_str(), header.getRemoteAddr().getHost().c_str(), header.getRemoteAddr().getPort());
	}
};

static void s_test_listener() {

	MySSDPPacketListener packetListener;
	SSDPMulticastListener ssdpListener("239.255.255.250", 1900);
	ssdpListener.setSSDPPacketHandler(&packetListener);
	ssdpListener.start();

	while (1) {
		ssdpListener.poll(100);
	}

	ssdpListener.stop();

}

int main(int argc, char * args[]) {

	s_test_listener();

	return 0;
}