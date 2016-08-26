#include <iostream>
#include <liboslayer/FileStream.hpp>
#include <libupnp-tools/SSDPServer.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace SSDP;

class MySSDPEventHandler : public SSDPEventHandler {
public:
	MySSDPEventHandler() {}
	virtual ~MySSDPEventHandler() {}

	virtual void onMsearchResponse(SSDPHeader & header) {
		cout << "RESP : " << header["LOCATION"] << endl;
	}
};

/**
 * @brief 
 */
int main(int argc, char *args[]) {

	bool done = false;
	SSDPServer server;

	server.startAsync();
	server.supportAsync(true);

	server.addSSDPEventHandler(AutoRef<SSDPEventHandler>(new MySSDPEventHandler));

	while (!done) {
		FileStream fs(stdin);
		string line;
		if ((line = fs.readline()) == "q") {
			done = true;
			break;
		}

		server.sendMsearchAsync(line, 3);
		
		// unsigned long tick = tick_milli();
		// cout << " ++ m-search : " << line << endl;
		// // server.sendMsearchAsync(line, 3);
		// AutoRef<SSDPMsearchSender> sender = server.sendMsearch(line, 3, AutoRef<Selector>(new Selector));
		// // AutoRef<SSDPMsearchSender> sender = server.sendMsearch(line, 3);
		// sender->gather(3 * 1000);
		// sender->close();
		// cout << " -- m-search : " << line << "(" << tick_milli() - tick << " ms.)" << endl;
	}

	server.stop();
    
    return 0;
}
