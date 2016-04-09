#include <iostream>
#include "utils.hpp"
#include <liboslayer/os.hpp>
#include <libupnp-tools/UPnPEventSubscriptionRegistry.hpp>
#include <libupnp-tools/UPnPNotificationServer.hpp>
#include <libupnp-tools/HttpUtils.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace UPNP;
using namespace HTTP;

static UPnPNotify s_notify;

class NotificationListener : public UPnPNotificationListener {
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

	UPnPNotificationServerConfig config(9998);
	UPnPNotificationServer server(config);
	
	server.startAsync();
	AutoRef<UPnPNotificationListener> listener(new NotificationListener);
	server.addNotificationListener(listener);

	UPnPEventSubscription subscription("uuid:xxxxx");
	server.addSubscription(subscription);

	send_notify(Url("http://localhost:9998/"));

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
