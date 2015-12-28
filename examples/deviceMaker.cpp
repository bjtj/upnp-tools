#include <iostream>
#include <string>
#include <liboslayer/os.hpp>
#include <liboslayer/Utils.hpp>
#include <libhttp-server/AnotherHttpClient.hpp>
#include <libupnp-tools/UPnPDevice.hpp>
#include <libupnp-tools/XmlDomParser.hpp>
#include <libupnp-tools/XmlNodeFinder.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace HTTP;
using namespace UPNP;
using namespace XML;

class DumpResponseHandler : public OnResponseListener {
private:
	std::string dump;
public:
    DumpResponseHandler() {}
    virtual ~DumpResponseHandler() {}
	virtual void onTransferDone(HttpResponse & response, DataTransfer * transfer, AutoRef<UserData> userData) {
        if (transfer) {
            dump = transfer->getString();
        }
    }
    virtual void onError(Exception & e, AutoRef<UserData> userData) {
        cout << "Error/e: " << e.getMessage() << endl;
    }
	string & getDump() {
		return dump;
	}
};


class UPnPDeviceBuilder {
private:
public:
    UPnPDeviceBuilder();
    virtual ~UPnPDeviceBuilder();
	UPnPDevice * build(const std::string & udn, const std::string & url);
	std::string getDump(Url & url);
};

UPnPDeviceBuilder::UPnPDeviceBuilder() {
}
UPnPDeviceBuilder::~UPnPDeviceBuilder() {
}
UPnPDevice * UPnPDeviceBuilder::build(const std::string & udn, const std::string & url) {
	AnotherHttpClient client;
	DumpResponseHandler handler;
	client.setOnResponseListener(&handler);
	client.setFollowRedirect(true);
	client.setUrl(url);
	client.setRequest("GET", LinkedStringMap(), NULL);
	client.execute();
	string dump = handler.getDump();
	XmlDomParser parser;
	XmlDocument doc = parser.parse(dump);
	XmlNode node = XmlNodeFinder::getNodeByTagName(doc.getRootNode(), "friendlyName");
	cout << "Friendly Name: " << node.getFirstContent() << endl;
	vector<XmlNode> scpds = XmlNodeFinder::getAllNodesByTagName(doc.getRootNode(), "SCPDURL");
	for (vector<XmlNode>::iterator iter = scpds.begin(); iter != scpds.end(); iter++) {
		Url u(url);
		u.setRelativePath(iter->getFirstContent());
		cout << "SCPD: " << u.toString() << endl;
		AnotherHttpClient client;
		DumpResponseHandler handler;
		client.setOnResponseListener(&handler);
		client.setUrl(u);
		client.setRequest("GET", LinkedStringMap(), NULL);
		client.execute();
		cout << handler.getDump() << endl;
	}
}

size_t readline(char * buffer, size_t max) {
    if (fgets(buffer, (int)max - 1, stdin)) {
        buffer[strlen(buffer) - 1] = 0;
        return strlen(buffer);
    }
    return 0;
}

void s_test() {
	bool done = false;
	while (!done) {
		char buffer[1024] = {0,};
		if (readline(buffer, sizeof(buffer)) > 0) {
			if (!strcmp(buffer, "q")) {
				done = true;
				break;
			} else {
				UPnPDeviceBuilder builder;
				builder.build("UDN", buffer);
			}
		}
	}
}

int main(int argc, char *args[]) {

	s_test();
    
    return 0;
}
