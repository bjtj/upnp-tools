#include <iostream>
#include <liboslayer/FileStream.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <libupnp-tools/Uuid.hpp>
#include <libupnp-tools/UPnPModels.hpp>
#include <libupnp-tools/SSDPMsearchSender.hpp>
#include <libupnp-tools/NetworkUtil.hpp>
#include <libupnp-tools/UPnPServer.hpp>
#include <libupnp-tools/UPnPActionHandler.hpp>
#include <libupnp-tools/UPnPDeviceDeserializer.hpp>
#include <libupnp-tools/UPnPResourceManager.hpp>
#include <libupnp-tools/UPnPDeviceProfileBuilder.hpp>

using namespace std;
using namespace SSDP;
using namespace HTTP;
using namespace UPNP;
using namespace UTIL;

string readline() {
	FileStream fs(stdin);
	return fs.readline();
}

string dd(const string & uuid) {
	string dummy = "urn:schemas-dummy-com:service:Dummy:1";
	string xml = "<?xml version=\"1.0\"?>\r\n"
		"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
		"<specVersion>"
		"<major>1</major>"
		"<minor>0</minor>"
		"</specVersion><device>"
		"<deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType>"
		"<friendlyName>UPnP Test Device</friendlyName>"
		"<manufacturer>Testers</manufacturer>"
		"<manufacturerURL>www.example.com</manufacturerURL>"
		"<modelDescription>UPnP Test Device</modelDescription>"
		"<modelName>UPnP Test Device</modelName>"
		"<modelNumber>1</modelNumber>"
		"<modelURL>www.example.com</modelURL>"
		"<serialNumber>12345678</serialNumber>"
		"<UDN>uuid:" + uuid + "</UDN>"
		"<serviceList>"
		"<service>"
		"<serviceType>urn:schemas-dummy-com:service:Dummy:1</serviceType>"
		"<serviceId>urn:dummy-com:serviceId:dummy1</serviceId>"
		"<controlURL>/control/" + uuid + "::" + dummy + "</controlURL>"
		"<eventSubURL>/event/" + uuid + "::" + dummy + "</eventSubURL>"
		"<SCPDURL>/scpd.xml/" + uuid + "::" + dummy + "</SCPDURL>"
		"</service></serviceList>"
		"</device>"
		"</root>";

	return xml;
}

string scpd() {

	string xml = "<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">"
		"<specVersion>"
		"<major>1</major>"
		"<minor>0</minor>"
		"</specVersion>"
		"<actionList>"
		"<action><name>GetProtocolInfo</name><argumentList><argument><name>Source</name><direction>out</direction><relatedStateVariable>SourceProtocolInfo</relatedStateVariable></argument><argument><name>Sink</name><direction>out</direction><relatedStateVariable>SinkProtocolInfo</relatedStateVariable></argument></argumentList></action>"
		"</actionList>"
		"<serviceStateTable>"
		"<stateVariable sendEvents=\"yes\"><name>SourceProtocolInfo</name><dataType>string</dataType></stateVariable><stateVariable sendEvents=\"yes\"><name>SinkProtocolInfo</name><dataType>string</dataType></stateVariable>"
		"</serviceStateTable>"
		"</scpd>";

	return xml;
}

class MyActionHandler : public UPnPActionHandler {
private:
public:
    MyActionHandler() {}
    virtual ~MyActionHandler() {}

	virtual void handleActionRequest(UPnPActionRequest & request, UPnPActionResponse & response) {

		cout << "** Action requst **" << endl;
		cout << " - service type : " << request.serviceType() << endl;
		cout << " - action name : " << request.actionName() << endl;
		
		if (request.actionName() == "GetProtocolInfo") {
			response["Source"] = "<sample source>";
			response["Sink"] = "<sample sink>";
		}
	}
};

int main(int argc, char *args[]) {

	UuidGeneratorDefault gen;
	string uuid = gen.generate();

	UPnPServer server(UPnPServer::Config(9001));
	server.startAsync();

	string dummy = "urn:schemas-dummy-com:service:Dummy:1";

	UPnPResourceManager::properties()["/device.xml"] = dd(uuid);
	UPnPResourceManager::properties()["/scpd.xml/" + uuid + "::" + dummy] = scpd();

	UPnPDeviceDeserializer deserializer;
	AutoRef<UPnPDevice> device = deserializer.build(Url("prop:///device.xml"));
	UPnPDeviceProfileBuilder builder(uuid, device);
	UPnPDeviceProfile deviceProfile = builder.build();

	// server[uuid] = deviceProfile;
	server.registerDeviceProfile(uuid, deviceProfile);

	LinkedStringMap props;
	props["SourceProtocolInfo"] = "<initial source>";
	props["SinkProtocolInfo"] = "<initial sink>";
	server.getNotificationCenter().registerService(uuid, dummy, props);
	
	AutoRef<UPnPActionHandler> handler(new MyActionHandler);
	server.setActionHandler(handler);

	cout << "uuid: " << uuid << endl;

	while (1) {
		string cmd;
		if ((cmd = readline()).size() > 0) {
			if (cmd == "q") {
				break;
			} else if (cmd == "alive") {
				server.getProfileManager().getDeviceProfileSessionWithUuid(uuid)->setEnable(true);
				server.notifyAlive(deviceProfile);
			} else if (cmd == "byebye") {
				server.getProfileManager().getDeviceProfileSessionWithUuid(uuid)->setEnable(false);
				server.notifyByeBye(deviceProfile);
			} else if (cmd == "set-props") {
				LinkedStringMap props;
				props["SourceProtocolInfo"] = "<sample sourc>";
				props["SinkProtocolInfo"] = "<sample sink>";
				server.getNotificationCenter().setProperties(uuid, dummy, props);
			} else if (cmd == "load") {
				// load mediaserver.lsp
				
			}
		}
	}

	server.stop();
    
    return 0;
}
