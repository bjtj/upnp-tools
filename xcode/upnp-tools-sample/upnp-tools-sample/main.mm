//
//  main.m
//  upnp-tools-sample
//
//  Created by TJ on 2015. 10. 22..
//  Copyright © 2015년 TJ. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <libupnp-tools/SSDPServer.hpp>


using namespace std;
using namespace SSDP;
using namespace HTTP;

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
        NSLog(@"notify - %s", header["Location"].c_str());
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

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // insert code here...
        NSLog(@"Hello, World!");
        
        s_test_ssdp_server();
    }
    return 0;
}
