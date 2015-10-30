#include <iostream>
#include <string>
#include <map>
#include <liboslayer/os.hpp>
#include <liboslayer/Text.hpp>
#include <libhttp-server/HttpClientThreadPool.hpp>
#include <libupnp-tools/SSDPServer.hpp>
#include <libupnp-tools/UPnPDevice.hpp>
#include <libupnp-tools/UPnPService.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace HTTP;
using namespace SSDP;
using namespace UPNP;

#define logd printf

static size_t readline(char * buffer, size_t max) {
	fgets(buffer, max - 1, stdin);
	buffer[strlen(buffer) - 1] = 0;
	return strlen(buffer);
}

static void printNotify(HttpHeader & header) {
	string nts = header["NTS"];
	if (Text::equalsIgnoreCase(nts, "ssdp:alive")) {
		logd("NOTIFY - %s (url: %s)\n", nts.c_str(), header["Location"].c_str());
	} else if (Text::equalsIgnoreCase(nts, "ssdp:byebye")) {
		logd("NOTIFY - %s\n", nts.c_str());
	} else {
		logd("NOTIFY - unknown (%s)\n", nts.c_str());
	}
}

static void printMsearch(HttpHeader & header) {
	logd("M-SEARCH - ST: %s\n", header["ST"].c_str());
}

static void printHttpResponse(HttpHeader & header) {
	logd("HTTP - %s (url: %s)\n", header["ST"].c_str(), header["Location"].c_str());
}

class SSDPMessageHandler : 
	public OnNotifyHandler, public OnMsearchHandler, public OnHttpResponseHandler {
private:
public:
	SSDPMessageHandler() {
	}
	virtual ~SSDPMessageHandler() {
	}

	virtual void onNotify(HttpHeader & header) {
		printNotify(header);
	}
	virtual void onMsearch(HttpHeader & header) {
		printMsearch(header);
	}
	virtual void onHttpResponse(HttpHeader & header) {
		printHttpResponse(header);
	}

};

class UPnPDeviceWriter : public UPnPDevice {
};

class DevicePool {
private:
	map<string, UPnPDevice> deviceTable;
public:
	DevicePool() {
	}
	virtual ~DevicePool() {
	}
	UPnPDevice & getDevice(string udn) {
	}
	void addDevice(UPnPDevice & device) {
		deviceTable[device.getUdn()] = device;
	}
	void removeDevice(string udn) {
		deviceTable.erase(udn);
	}
};


int main(int argc, char * args[]) {

	SSDPMessageHandler ssdpHandler;
	SSDPServer ssdp;

	ssdp.addHttpResponseHandler(&ssdpHandler);
	ssdp.addNotifyHandler(&ssdpHandler);
	ssdp.addMsearchHandler(&ssdpHandler);

	HttpClientThreadPool<string> httpClientPool(10);
	httpClientPool.start();

	ssdp.startAsync();

	while (1) {
		char buffer[1024] = {0,};
		readline(buffer, sizeof(buffer));
		if (!strcmp(buffer, "q")) {
			break;
		}
		if (!strcmp(buffer, "m")) {
			ssdp.sendMsearch("upnp:rootdevice");
		}
	}

	httpClientPool.stop();
	ssdp.stop();

	logd("exit\n");

	return 0;
}