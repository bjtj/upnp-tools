#include <iostream>
#include <liboslayer/FileStream.hpp>
#include <libupnp-tools/SSDPServer.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace SSDP;

class MySSDPEventListener : public SSDPEventListener {
public:
	MySSDPEventListener() {}
	virtual ~MySSDPEventListener() {}

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

	server.addSSDPEventListener(AutoRef<SSDPEventListener>(new MySSDPEventListener));

	while (!done) {
		FileStream fs(stdin);
		string line;
		if ((line = fs.readline()) == "q") {
			done = true;
			break;
		}

		server.sendMsearchAsync(line, 3);
	}

	server.stop();
    
    return 0;
}
