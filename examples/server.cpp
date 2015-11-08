#include <iostream>
#include <string>
#include <map>
#include <liboslayer/Text.hpp>
#include <libupnp-tools/UPnPServer.hpp>

using namespace std;
using namespace UTIL;
using namespace UPNP;

size_t readline(char * buffer, size_t max) {
    fgets(buffer, (int)max - 1, stdin);
    buffer[strlen(buffer) - 1] = 0;
    return strlen(buffer);
}

int main(int argc, char * args[]) {
    
    bool done = false;
    char buffer[1024] = {0,};
    UPnPServer server(8083);
    
    UPnPDevice device;
    device.setUdn("uuid:fc4ec57e-b051-11db-88f8-006008abcdef");
    device.setFriendlyName("Dummy Device");
    server.registerDevice(device);
    
    server.startAsync();
    
    while (!done) {
        readline(buffer, sizeof(buffer));
        if (!strcmp(buffer, "q")) {
            done = true;
            break;
        }
        if (!strcmp(buffer, "a")) {
            server.announceDevice(server.getDevice("uuid:fc4ec57e-b051-11db-88f8-006008abcdef"));
        }
    }
    
    server.stop();
    
    return 0;
}