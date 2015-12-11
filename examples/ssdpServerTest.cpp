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
        virtual bool filter(SSDP::SSDPHeader & header) {
            if (!header.getRemoteAddr().getHost().compare("192.168.0.1")) {
                return false;
            }
            if (!header.getRemoteAddr().getHost().compare("192.168.0.2")) {
                return false;
            }
            return true;
        }
		virtual void onMsearch(SSDP::SSDPHeader & header) {
            OS::InetAddress addr = header.getRemoteAddr();
			printf("MSEARCH / ST: %s (%s:%d)\n", header.getSt().c_str(), addr.getHost().c_str(), addr.getPort());
		}
		virtual void onNotify(SSDP::SSDPHeader & header) {
            OS::InetAddress addr = header.getRemoteAddr();
			if (header.isNotifyAlive()) {
				printf("NOTIFY / alive :: URL: %s (%s:%d)\n", header.getLocation().c_str(), addr.getHost().c_str(), addr.getPort());
			} else {
				printf("NOTIFY / byebye :: %s (%s:%d)\n", header.getNt().c_str(), addr.getHost().c_str(), addr.getPort());
			}
		}
		virtual void onMsearchResponse(SSDP::SSDPHeader & header) {
            OS::InetAddress addr = header.getRemoteAddr();
			printf("RESP / URL: %s (%s:%d)\n", header.getLocation().c_str(), addr.getHost().c_str(), addr.getPort());
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