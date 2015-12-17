#include <iostream>
#include <liboslayer/os.hpp>
#include <liboslayer/Utils.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <libhttp-server/AnotherHttpClient.hpp>
#include <libupnp-tools/AnotherSSDPServer.hpp>
#include <libupnp-tools/UPnPDevicePool.hpp>
#include <libupnp-tools/Uuid.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace HTTP;
using namespace SSDP;
using namespace UPNP;

class SSDPHandler : public SSDPPacketHandler {
private:
public:
	SSDPHandler() {}
	virtual ~SSDPHandler() {}
	virtual bool filter(SSDPHeader & header) {
		return true;
	}
	virtual void onNotify(SSDPHeader & header) {
		Uuid uuid(header["USN"]);
		cout << header["NTS"] << endl;
		cout << " - UUID: " << uuid.getUuid() << endl;
		cout << " - TYPE: " << uuid.getRest() << endl;
		if (header.isNotifyAlive()) {
			cout << " - Location: " << header["Location"] << endl;
		}
	}
	virtual void onMsearchResponse(SSDPHeader & header) {}
};

class PollThread : public Thread {
private:
	AnotherSSDPServer * server;
public:
    PollThread(AnotherSSDPServer * server) : server(server) {}
    virtual ~PollThread() {}

	virtual void run() {
		while (!interrupted()) {
			server->poll(100);
		}
	}
};

size_t readline(char * buffer, size_t max) {
    if (fgets(buffer, (int)max - 1, stdin)) {
        buffer[strlen(buffer) - 1] = 0;
        return strlen(buffer);
    }
    return 0;
}

int main(int argc, char *args[]) {

	bool done = false;
	AnotherSSDPServer server;
	
	SSDPHandler ssdpHandler;
	server.setSSDPPacketHandler(&ssdpHandler);

	server.start();

	PollThread thread(&server);
	thread.start();

	while(!done) {
		char buffer[1024] = {0,};
		if (readline(buffer, sizeof(buffer)) > 0) {
			if (!strcmp(buffer, "q")) {
				break;
			}
		}
	}

	thread.interrupt();
	thread.join();
	server.stop();

	cout << "done." << endl;
    
    return 0;
}
