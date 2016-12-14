#include <iostream>
#include <liboslayer/FileStream.hpp>
#include <libupnp-tools/SSDPServer.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace SSDP;

class Debug {
private:
	bool _verbose;
public:
    Debug() : _verbose(false) {}
    virtual ~Debug() {}
	bool & verbose() {return _verbose;}
};


class MySSDPEventListener : public SSDPEventListener {
private:
	Debug & debug;
public:
	MySSDPEventListener(Debug & debug) : debug(debug) {}
	virtual ~MySSDPEventListener() {}

	virtual void onMsearch(SSDPHeader & header) {
		cout << "M-SEARCH : " << header["ST"] << endl;
		if (debug.verbose()) {
			cout << " ** " << header.toString() << endl;
		}
	}

	virtual void onNotify(SSDPHeader & header) {
		cout << "NOTIFY : " << header["NTS"] << " - " << header["LOCATION"] << endl;
		if (debug.verbose()) {
			cout << " ** " << header.toString() << endl;
		}
	}

	virtual void onMsearchResponse(SSDPHeader & header) {
		cout << "RESP : " << header["LOCATION"] << endl;
		if (debug.verbose()) {
			cout << " ** " << header.toString() << endl;
		}
	}
};

/**
 * @brief 
 */
int main(int argc, char *args[]) {

	bool done = false;
	SSDPServer server;
	Debug debug;

	server.startAsync();
	server.supportMsearchAsync(true);

	server.addSSDPEventListener(AutoRef<SSDPEventListener>(new MySSDPEventListener(debug)));

	while (!done) {
		FileStream fs(stdin);
		string line = fs.readline();
		if (line == "q") {
			done = true;
			break;
		} else if (line == "v") {
			debug.verbose() = !debug.verbose();
		} else {
			server.sendMsearchAsync(line, 3);
		}
	}

	server.stop();
    
    return 0;
}
