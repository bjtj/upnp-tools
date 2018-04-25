#include <iostream>
#include "utils.hpp"
#include <liboslayer/os.hpp>
#include <libupnp-tools/UPnPEventReceiver.hpp>
#include <libupnp-tools/HttpUtils.hpp>

using namespace std;
using namespace osl;
using namespace http;
using namespace upnp;


static UPnPNotify s_notify;

class NotificationListener : public UPnPEventListener {
private:
public:
    NotificationListener() {
	}
    virtual ~NotificationListener() {
	}
	virtual void onNotify(UPnPNotify & notify) {
		s_notify = notify;
		cout << "NOTIFY: " << notify.sid() << endl;
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

	ASSERT(s_notify.sid(), ==, "uuid:xxxxx");
	ASSERT(s_notify["SystemUpdateID"], ==, "72826040");

	server.stop();
}

static void test_subscription_registry() {

	UPnPEventSubscriptionRegistry registry;

	UPnPEventSubscription subscription("uuid:xxxxx");
	registry.addSubscription(subscription);

	ASSERT(registry["uuid:xxxxx"].sid(), ==, "uuid:xxxxx");
	ASSERT(registry["uuid:xxxxx"].lastSeq(), ==, 0);
}

int main(int argc, char *args[]) {

	test_notification_server();
	test_subscription_registry();
    
    return 0;
}
