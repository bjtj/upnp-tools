#include <iostream>
#include "SSDPServer.hpp"
#include "os.hpp"

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
		std::cout << "msearch" << std::endl;
	}

	virtual void onNotify(HttpHeader & header) {
		std::cout << "notify" << std::endl;
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

int main(int argc, char *args[]) {

	s_test_ssdp_server();

	std::cout << "Done" << std::endl;
    
    return 0;
}
