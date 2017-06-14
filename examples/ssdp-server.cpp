#include <iostream>
#include <liboslayer/FileStream.hpp>
#include <libupnp-tools/SSDPServer.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace SSDP;

/**
 * 
 */
class SimpleSSDPEventListener : public SSDPEventListener {
private:
public:
	SimpleSSDPEventListener() {}
	virtual ~SimpleSSDPEventListener() {}

	virtual void onMsearch(SSDPHeader & header) {
		cout << "[RECV] M-SEARCH / search target: '" << header["ST"] << "' - " << header.getRemoteAddr().toString() << endl;
	}
};

int main(int argc, char *args[]) {

	bool done = false;
	SSDPServer server;

	server.startAsync();
	server.addSSDPEventListener(AutoRef<SSDPEventListener>(new SimpleSSDPEventListener));

	while (!done) {
		FileStream fs(stdin);
		string line = fs.readline();
		if (line == "q" || line == "quit") {
			break;
		}
	}

	server.stop();
    
    return 0;
}
