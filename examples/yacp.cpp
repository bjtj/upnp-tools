#include <iostream>
#include <liboslayer/os.hpp>
#include <liboslayer/Utils.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <libhttp-server/AnotherHttpClient.hpp>
#include <libhttp-server/FixedTransfer.hpp>
#include <libupnp-tools/AnotherSSDPServer.hpp>
#include <libupnp-tools/UPnPDevicePool.hpp>
#include <libupnp-tools/Uuid.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace HTTP;
using namespace SSDP;
using namespace UPNP;

class HttpResponseListener : public OnResponseListener {
private:
public:
	HttpResponseListener() {}
	virtual ~HttpResponseListener() {}
	virtual void onResponseHeader(HttpResponse & response, UTIL::AutoRef<UserData> userData) {
		response.setTransfer(createDataTransfer(response.getHeader()));
	}
	virtual void onTransferDone(HttpResponse & response, DataTransfer * transfer, UTIL::AutoRef<UserData> userData) {
		if (transfer) {
			cout << transfer->getString() << endl;
		}
	}
	virtual void onError(OS::Exception & e, UTIL::AutoRef<UserData> userData) {

	}
};

class SSDPHandler : public SSDPPacketHandler {
private:
public:
	SSDPHandler() {}
	virtual ~SSDPHandler() {}
	virtual bool filter(SSDPHeader & header) {
		return true;
	}
	void requestDeviceDescription(const std::string & url) {
		AnotherHttpClient client;
		HttpResponseListener listener;
		client.setOnResponseListener(&listener);
		client.setUrl(url);
		client.setRequest("GET", UTIL::LinkedStringMap(), NULL);
		client.execute();
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
	virtual void onMsearchResponse(SSDPHeader & header) {
		Uuid uuid(header["USN"]);
		cout << "RESP:" << endl;
		cout << " - UUID: " << uuid.getUuid() << endl;
		cout << " - TYPE: " << uuid.getRest() << endl;
		cout << " - Location: " << header["Location"] << endl;
		if (!uuid.getRest().compare("upnp:rootdevice")) {
			// requestDeviceDescription(header["Location"]);
			// TODO: apply it after connection timeout check implementated
		}
	}
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
			if (!strcmp(buffer, "m")) {
				vector<string> st;
				st.push_back("upnp:rootdevice");
				st.push_back("ssdp:all");
				server.sendMsearchAndGather(st, 5);
				cout << "msearch gathering done." << endl;
			}
		}
	}

	thread.interrupt();
	thread.join();
	server.stop();

	cout << "done." << endl;
    
    return 0;
}
