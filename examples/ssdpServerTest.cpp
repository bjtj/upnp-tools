#include <libupnp-tools/AnotherSSDPServer.hpp>

size_t readline(char * buffer, size_t max) {
    if (fgets(buffer, (int)max - 1, stdin)) {
		buffer[strlen(buffer) - 1] = 0;
		return strlen(buffer);
	}
    return 0;
}

class PollingThread : public OS::Thread {
private:
    SSDP::AnotherSSDPServer * server;
public:
    PollingThread(SSDP::AnotherSSDPServer * server) : server(server) {
    }
    virtual ~PollingThread() {
    }
    
    virtual void run() {
        while (!interrupted()) {
            server->poll(100);
        }
    }
};

int main(int argc, char * args[]) {

	SSDP::AnotherSSDPServer server;

	class MySSDPPacketHandler : public SSDP::SSDPPacketHandler {
	private:
	public:
		MySSDPPacketHandler() {
		}
		virtual ~MySSDPPacketHandler() {
		}
		virtual void onMsearch(SSDP::SSDPHeader & header) {
			printf("MSEARCH / ST: %s\n", header.getSt().c_str());
		}
		virtual void onNotify(SSDP::SSDPHeader & header) {
			if (header.isNotifyAlive()) {
				printf("NOTIFY / URL: %s\n", header.getLocation().c_str());
			} else {
				printf("NOTIFY / %s\n", header.getNt().c_str());
			}
		}
		virtual void onMsearchResponse(SSDP::SSDPHeader & header) {
			printf("RESP / URL: %s\n", header.getLocation().c_str());
		}
	};

	MySSDPPacketHandler handler;

	server.setSSDPPacketHandler(&handler);

	server.start();
    
    PollingThread th(&server);
    th.start();

	while (1) {
		char buffer[1024] = {0,};
		if (readline(buffer, sizeof(buffer)) > 0) {
			if (!strcmp(buffer, "q")) {
				break;
			}

			server.sendMsearch(buffer, 3);
		}
	}
    
    th.interrupt();
    th.join();

	server.stop();

	return 0;
}