#include <libupnp-tools/SSDPServer.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>
#include <libhttp-server/HttpClient.cpp>
#include <string>
#include <iostream>

using namespace std;
using namespace SSDP;
using namespace UPNP;
using namespace HTTP;

class MyHttpResponseHandler : public HttpResponseHandler {
private:
public:
    MyHttpResponseHandler() {}
    virtual ~MyHttpResponseHandler() {}
    
    virtual void onResponse(HttpClient & client, HttpHeader & responseHeader, Socket & socket) {
        
        char buffer[1024] = {0,};
        int contentLength = responseHeader.getContentLength();
        int total = 0;
        int len;
        while (contentLength > total && (len = socket.recv(buffer, sizeof(buffer))) > 0) {
            string msg(buffer, len);
            printf("%s\n", msg.c_str());
            total += len;
        }
    }
};

class SSDPHandler : public OnMsearchHandler, public OnNotifyHandler {
private:
public:
    SSDPHandler() {
    }
    virtual ~SSDPHandler() {
    }
    
    virtual void onMsearch(HttpHeader & header) {
        printf("msearch - %s\n", header["ST"].c_str());
    }
    
    virtual void onNotify(HttpHeader & header) {
        string location = header["Location"];
        printf("notify - %s\n", location.c_str());
        if (!location.empty()) {
            HttpClient client;
            Url url(location);
            MyHttpResponseHandler handler;
            client.setHttpResponseHandler(&handler);
            client.request(url);
        }
    }
};

static void s_test_ssdp_server() {
    SSDPServer server;
    
    SSDPHandler handler;
    
    server.startAsync();
    server.addNotifyHandler(&handler);
    server.addMsearchHandler(&handler);
    
    getchar();
    
    server.stop();
}

class MyDeviceAddRemoveHandle : public OnDeviceAddRemoveListener {
private:
public:
	MyDeviceAddRemoveHandle() {}
	virtual ~MyDeviceAddRemoveHandle() {}

	virtual void onDeviceAdd(UPnPDevice & device) {
		cout << "[Device Added] udn: " << device["UDN"] << endl;
	}
	virtual void onDeviceRemove(UPnPDevice & device) {
	}
};

static void s_test_cp() {
    UPnPControlPoint cp(1900, "ssdp:all");

	MyDeviceAddRemoveHandle listener;
	cp.setOnDeviceAddRemoveListener(&listener);

    cp.startAsync();
    
    getchar();
    
    cp.stop();
}

static void s_device_builder_test() {
	// request device description
	// dd arrived
	// reigster builder
	// request scpd
	// scpd arrived
	// fill the services
	// if all scpd arrived finish builder
}

int main(int argc, const char * argv[]) {

	// s_test_ssdp_server();
	s_test_cp();

    return 0;
}
