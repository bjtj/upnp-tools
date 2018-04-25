#include <iostream>
#include <liboslayer/FileStream.hpp>
#include <libupnp-tools/SSDPServer.hpp>

using namespace std;
using namespace osl;
using namespace ssdp;

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

	virtual void onMsearch(const SSDPHeader & header) {
		cout << "[M-SEARCH] " << header.getSearchTarget()
			 << " from " << header.getRemoteAddr().toString() << endl;
		if (debug.verbose()) {
			cout << " ** " << header.toString() << endl;
		}
	}

	virtual void onNotify(const SSDPHeader & header) {
		cout << "[NOTIFY] " << header.getNotificationSubType()
			 << " | " << header.getLocation()
			 << " from " << header.getRemoteAddr().toString()<< endl;
		if (debug.verbose()) {
			cout << " ** " << header.toString() << endl;
		}
	}

	virtual void onMsearchResponse(const SSDPHeader & header) {
		cout << "[RESP] " << header.getLocation()
			 << " from " << header.getRemoteAddr().toString()<< endl;
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
			cout << "[VERBOSE MODE] " << (debug.verbose() ? "ON" : "OFF") << endl;
		} else {
			server.sendMsearchAsync(line, 3);
		}
	}

	server.stop();
    
    return 0;
}
