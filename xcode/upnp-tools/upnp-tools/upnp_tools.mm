#import "upnp_tools.h"
#import "UPnPServer.hpp"

using namespace UPNP;

@implementation upnp_tools
{
    UPnPServer * server;
}

- (void)hello
{
    NSLog(@"hello");
}

- (void)startServer
{
    if (!server) {
        server = new UPnPServer(8080);
        server->startAsync();
    }
}

- (void)stopServer
{
    if (server) {
        server->stop();
        delete server;
        server = NULL;
    }
}

@end
