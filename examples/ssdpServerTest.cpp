#include <iostream>
#include <libupnp-tools/SSDPServer.hpp>

using namespace std;


size_t readline(char * buffer, size_t max) {
    if (fgets(buffer, (int)max - 1, stdin)) {
		buffer[strlen(buffer) - 1] = 0;
		return strlen(buffer);
	}
    return 0;
}

class MySSDPPacketHandler : public SSDP::SSDPEventHandler {
private:
public:
	MySSDPPacketHandler() {
	}
	virtual ~MySSDPPacketHandler() {
	}
	virtual bool filter(SSDP::SSDPHeader & header) {
		return true;
	}
	virtual void onMsearch(SSDP::SSDPHeader & header) {
		OS::InetAddress addr = header.getRemoteAddr();
		printf("MSEARCH / ST: %s (%s:%d)\n",
			   header.getSt().c_str(), addr.getHost().c_str(), addr.getPort());
	}
	virtual void onNotify(SSDP::SSDPHeader & header) {
		OS::InetAddress addr = header.getRemoteAddr();
		if (header.isNotifyAlive()) {
			printf("NOTIFY / alive :: URL: %s (%s:%d)\n",
				   header.getLocation().c_str(), addr.getHost().c_str(), addr.getPort());
		} else {
			printf("NOTIFY / byebye :: %s (%s:%d)\n",
				   header.getNt().c_str(), addr.getHost().c_str(), addr.getPort());
		}
	}
	virtual void onMsearchResponse(SSDP::SSDPHeader & header) {
		OS::InetAddress addr = header.getRemoteAddr();
		printf("RESP / ST: %s / URL: %s (%s:%d)\n",
			   header.getSt().c_str(), header.getLocation().c_str(), addr.getHost().c_str(), addr.getPort());
	}
};

int main(int argc, char * args[]) {

	SSDP::SSDPServer server;
	
	MySSDPPacketHandler handler;

	server.setSSDPEventHandler(&handler);
	server.startAsync(100);
    
	while (1) {
		char buffer[1024] = {0,};
		if (readline(buffer, sizeof(buffer)) > 0) {
			if (!strcmp(buffer, "q")) {
				break;
			}

			cout << "## M-SEARCH in 3 seconds." << endl;
			server.sendMsearchAndGather(buffer, 3);
			cout << "## M-SEARCH done" << endl;
		}
	}
    
	server.stop();

	return 0;
}
