#include <iostream>
#include <liboslayer/FileStream.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <liboslayer/Uuid.hpp>
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

/**
 * @brief 
 */
string readline() {
	FileStream fs(stdin);
	return fs.readline();
}

/**
 * @brief 
 */
string dd(const string & uuid) {
	string dummy = "urn:schemas-dummy-com:service:Dummy:1";
	string xml = "<?xml version=\"1.0\" charset=\"utf-8\"?>\r\n";
	xml.append("<root xmlns=\"urn:schemas-upnp-org:device-1-0\">");
	xml.append("<specVersion>");
	xml.append("<major>1</major>");
	xml.append("<minor>0</minor>");
	xml.append("</specVersion><device>");
	xml.append("<deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType>");
	xml.append("<friendlyName>UPnP Test Device</friendlyName>");
	xml.append("<manufacturer>Testers</manufacturer>");
	xml.append("<manufacturerURL>www.example.com</manufacturerURL>");
	xml.append("<modelDescription>UPnP Test Device</modelDescription>");
	xml.append("<modelName>UPnP Test Device</modelName>");
	xml.append("<modelNumber>1</modelNumber>");
	xml.append("<modelURL>www.example.com</modelURL>");
	xml.append("<serialNumber>12345678</serialNumber>");
	xml.append("<UDN>uuid:" + uuid + "</UDN>");
	xml.append("<serviceList>");
	xml.append("<service>");
	xml.append("<serviceType>urn:schemas-dummy-com:service:Dummy:1</serviceType>");
	xml.append("<serviceId>urn:dummy-com:serviceId:dummy1</serviceId>");
	xml.append("<controlURL>/control/");
	xml.append(uuid);
	xml.append("::");
	xml.append(dummy);
	xml.append("</controlURL>");
	xml.append("<eventSubURL>/event/");
	xml.append(uuid);
	xml.append("::");
	xml.append(dummy);
	xml.append("</eventSubURL>");
	xml.append("<SCPDURL>/scpd.xml/");
	xml.append(uuid);
	xml.append("::");
	xml.append(dummy);
	xml.append("</SCPDURL>");
	xml.append("</service></serviceList>");
	xml.append("</device>");
	xml.append("</root>");

	return xml;
}

/**
 * @brief 
 */
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

/**
 * @brief 
 */
class MyActionHandler : public UPnPActionRequestHandler {
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

/**
 * @brief 
 */
static void s_set_dummy(UPnPServer & server, const string & uuid) {
	
	string dummy = "urn:schemas-dummy-com:service:Dummy:1";

	UPnPResourceManager::properties()["/device.xml"] = dd(uuid);
	UPnPResourceManager::properties()["/scpd.xml/" + uuid + "::" + dummy] = scpd();

	UPnPDeviceDeserializer deserializer;
	AutoRef<UPnPDevice> device = deserializer.build(Url("prop:///device.xml"));
	UPnPDeviceProfileBuilder builder(uuid, device);
	UPnPDeviceProfile deviceProfile = builder.build();

	server.registerDeviceProfile(uuid, deviceProfile);

	LinkedStringMap props;
	props["SourceProtocolInfo"] = "<initial source>";
	props["SinkProtocolInfo"] = "<initial sink>";
	server.getNotificationCenter().registerService(uuid, dummy, props);
}

/**
 * @brief 
 */
int main(int argc, char *args[]) {

	UuidGeneratorVersion1 gen;
	string uuid = gen.generate();

	UPnPServer server(UPnPServer::Config(9001));
	server.startAsync();

	s_set_dummy(server, uuid);
	
	AutoRef<UPnPActionRequestHandler> handler(new MyActionHandler);
	server.setActionRequestHandler(handler);

	cout << "uuid: " << uuid << endl;

	while (1) {
		string cmd;
		if ((cmd = readline()).size() > 0) {
			if (cmd == "q") {
				break;
			}

			vector<string> tokens = Text::split(cmd, " ");

			if (tokens.size() == 0) {
				continue;
			}

			if (tokens[0] == "alive") {
				cout << " * alive : " << uuid << endl;
				server.getProfileManager().getDeviceProfileSessionWithUuid(uuid)->setEnable(true);
				server.notifyAlive(server.getProfileManager().getDeviceProfileSessionWithUuid(uuid)->profile());
			} else if (tokens[0] == "byebye") {
				cout << " * byebye : " << uuid << endl;
				server.getProfileManager().getDeviceProfileSessionWithUuid(uuid)->setEnable(false);
				server.notifyByeBye(server.getProfileManager().getDeviceProfileSessionWithUuid(uuid)->profile());
			} else if (tokens[0] == "list") {
				vector<AutoRef<UPnPDeviceProfileSession> > vec = server.getProfileManager().sessionList();
				for (size_t i = 0; i < vec.size(); i++) {
					cout << "[" << i << "] " << vec[i]->profile().uuid() << " ; " << (vec[i]->profile().deviceTypes().size() > 0 ? vec[i]->profile().deviceTypes()[0] : "") << endl;
				}
			} else if (tokens[0] == "set-props") {
				LinkedStringMap props;
				props["SourceProtocolInfo"] = "<sample source>";
				props["SinkProtocolInfo"] = "<sample sink>";
				server.getNotificationCenter().setProperties(uuid, "urn:schemas-dummy-com:service:Dummy:1", props);
			} else if (tokens[0] == "load") {
				if (tokens.size() < 2) {
					continue;
				}
				string uri = tokens[1];
			} else if (tokens[0] == "unload") {
				if (tokens.size() < 2) {
					continue;
				}
				string uuid = tokens[1];
			}
		}
	}

	server.stop();
    
    return 0;
}
