#include <iostream>
#include <liboslayer/os.hpp>
#include <libupnp-tools/SSDPServer.hpp>

using namespace std;
using namespace SSDP;
using namespace HTTP;

class SSDPHandler : public OnMsearchHandler, public OnNotifyHandler, public OnHttpResponseHandler {
private:
public:
    SSDPHandler() {
	}
    virtual ~SSDPHandler() {
	}

	virtual void onMsearch(const HttpHeader & header, const OS::InetAddress & remoteAddr) {
		std::cout << "msearch - " << header.getHeaderFieldIgnoreCase("ST") << std::endl;
	}

	virtual void onNotify(const HttpHeader & header) {
		std::cout << "notify - " << header.getHeaderFieldIgnoreCase("NTS") << " . " << header.getHeaderFieldIgnoreCase("Location") << std::endl;
	}

	virtual void onHttpResponse(const HttpHeader & header) {
		std::cout << "response - " << header.getHeaderFieldIgnoreCase("Location") << std::endl;
	}
};


static void s_test_ssdp_server() {
	SSDPServer server;

	SSDPHandler handler;

	server.startAsync();
	server.addNotifyHandler(&handler);
	server.addMsearchHandler(&handler);
	server.addHttpResponseHandler(&handler);

	while (1) {
		char buffer[1024] = {0,};
		if (fgets(buffer, sizeof(buffer) - 1, stdin)) {
			buffer[strlen(buffer) - 1] = 0;
			if (!strcmp(buffer, "q")) {
				break;
			}
			if (!strcmp(buffer, "m")) {
				server.sendMsearch("upnp:rootdevice");
			}
		}
	}

	server.stop();
}

int main(int argc, char *args[]) {

	s_test_ssdp_server();

	std::cout << "Done" << std::endl;
    
    return 0;
}
