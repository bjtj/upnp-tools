#include <iostream>
#include "utils.hpp"
#include <liboslayer/os.hpp>
#include <libupnp-tools/UPnPEventReceiver.hpp>
#include <libupnp-tools/HttpUtils.hpp>

using namespace std;
using namespace osl;
using namespace http;
using namespace upnp;


static UPnPPropertySet s_propset;

class NotificationListener : public UPnPEventListener {
private:
public:
    NotificationListener() {
	}
    virtual ~NotificationListener() {
	}
	virtual void onNotify(UPnPPropertySet & propset) {
		s_propset = propset;
		cout << "Property: " << propset.sid() << endl;
	}
};

static void send_notify(Url url) {

	LinkedStringMap headers;
	headers["SID"] = "uuid:xxxxx";
	headers["SEQ"] = "0";
	headers["NT"] = "upnp:event";
	
	string content;
	content.append("<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\">");
	content.append("<e:property>");
	content.append("<SystemUpdateID>72826040</SystemUpdateID>");
	content.append("</e:property>");
	content.append("</e:propertyset>");
	HttpUtils::httpPost(url, headers, content);
}

static void test_notification_server() {

	UPnPEventReceiverConfig config(9002);
	UPnPEventReceiver server(config);
	
	server.startAsync();
	AutoRef<UPnPEventListener> listener(new NotificationListener);
	server.addEventListener(listener);

	UPnPEventSubscription subscription("uuid:xxxxx");
	server.addSubscription(subscription);

	send_notify(Url("http://127.0.0.1:9002/"));

	idle(1000);

	ASSERT(s_propset.sid(), ==, "uuid:xxxxx");
	ASSERT(s_propset["SystemUpdateID"], ==, "72826040");

	server.stop();
}


int main(int argc, char *args[]) {

	test_notification_server();

    return 0;
}
