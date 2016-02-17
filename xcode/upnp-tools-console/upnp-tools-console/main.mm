//
//  main.m
//  upnp-tools-sample
//
//  Created by TJ on 2015. 10. 22..
//  Copyright © 2015년 TJ. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <libupnp-tools/SSDPServer.hpp>
#import <libupnp-tools/UPnPControlPoint.hpp>
#import <libhttp-server/HttpClient.cpp>
#import <string>

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
            NSLog(@"%s", msg.c_str());
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
        NSLog(@"msearch - %s", header["ST"].c_str());
    }
    
    virtual void onNotify(HttpHeader & header) {
        string location = header["Location"];
        NSLog(@"notify - %s", location.c_str());
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

static void s_test_cp() {
    UPnPControlPoint cp(1900, "ssdp:all");
    cp.startAsync();
    
    getchar();
    
    cp.stop();
}

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // insert code here...
        NSLog(@"Hello, World!");
        
        // s_test_ssdp_server();
        s_test_cp();
    }
    return 0;
}
