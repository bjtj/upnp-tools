#include <liboslayer/os.hpp>
#include <liboslayer/DatagramSocket.hpp>
#include <liboslayer/Text.hpp>
#include <libhttp-server/HttpHeader.hpp>
#include <libhttp-server/HttpHeaderReader.hpp>
#include <libupnp-tools/SSDPHeader.hpp>
#include <libupnp-tools/SSDPPacketHandler.hpp>
#include <libupnp-tools/SSDPMsearchSender.hpp>

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

static void s_test_msearch(const std::string & st) {

	MySSDPPacketListener packetListener;
	SSDPMsearchSender sender;
	sender.setSSDPPacketHandler(&packetListener);

	unsigned long mx = 5;
	sender.sendMsearchAllInterfaces(st, mx, "239.255.255.250", 1900);
	sender.gather(mx * 1000);

	sender.close();
}

int main(int argc, char * args[]) {

	while (1) {
		s_test_msearch("upnp:rootdevice");
		printf("press q to quit: ");
		int ch = getchar();
		if (ch == 'q') {
			break;
		}
	}

	getchar();

	return 0;
}