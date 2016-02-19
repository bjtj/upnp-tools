#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <liboslayer/AutoRef.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>

using namespace std;
using namespace UTIL;
using namespace UPNP;

int run(int argc, char *args[]);

int main(int argc, char *args[]) {
	return run(argc, args);
}

size_t readline(char * buffer, size_t max) {
    if (fgets(buffer, (int)max - 1, stdin)) {
		buffer[strlen(buffer) - 1] = 0;
		return strlen(buffer);
	}
    return 0;
}

size_t prompt(const string & msg, char * buffer, size_t max) {
	cout << msg;
	return readline(buffer, max);
}

void printList(UPnPSessionManager & sessionManager) {
	cout << " == Device List (" << sessionManager.getUdnS().size() << ") ==" << endl;
	vector<string> lst = sessionManager.getUdnS();
	size_t i = 0;
	for (vector<string>::iterator iter = lst.begin(); iter != lst.end(); iter++, i++) {
		AutoRef<UPnPSession> session = sessionManager[*iter];
		cout << " [" << i << "] " << session->toString() << endl;
	}
}

int run(int argc, char *args[]) {

	UPnPControlPoint cp;

	cp.startAsync();

	while (1) {
		char buffer[1024] = {0,};
		if (readline(buffer, sizeof(buffer)) > 0) {
			if (!strcmp(buffer, "q")) {
				break;
			} else if (!strcmp(buffer, "clear")) {
				cp.clearDevices();
			} else {
				cout << " ** Searching... **" << endl;
				cp.sendMsearchAndWait(buffer, 3);
				cout << " ** Searching Done **" << endl << endl;
			}
		} else {
			printList(cp.sessionManager());
		}
	}

	cp.stop();
    
    return 0;
}
