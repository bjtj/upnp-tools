#include <iostream>
#include <liboslayer/os.hpp>
#include <liboslayer/ArgumentParser.hpp>
#include <liboslayer/FileStream.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <liboslayer/Uuid.hpp>
#include <libupnp-tools/UPnPModels.hpp>
#include <libupnp-tools/SSDPMsearchSender.hpp>
#include <libupnp-tools/NetworkUtil.hpp>
#include <libupnp-tools/UPnPServer.hpp>
#include <libupnp-tools/UPnPActionRequestHandler.hpp>
#include <libupnp-tools/UPnPDeviceDeserializer.hpp>
#include <libupnp-tools/UPnPResourceManager.hpp>
#include <libupnp-tools/UPnPDeviceProfileBuilder.hpp>

using namespace std;
using namespace OS;
using namespace SSDP;
using namespace HTTP;
using namespace UPNP;
using namespace UTIL;

static bool s_lightOn = false;
static int s_level = 100;

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
	string svc_sp1 = "urn:schemas-upnp-org:service:SwitchPower:1";
	string svc_d1 = "urn:schemas-upnp-org:service:Dimming:1";
	string xml = "<?xml version=\"1.0\" charset=\"utf-8\"?>\r\n";
	xml.append("<root xmlns=\"urn:schemas-upnp-org:device-1-0\">");
	xml.append("<specVersion>");
	xml.append("<major>1</major>");
	xml.append("<minor>0</minor>");
	xml.append("</specVersion><device>");
	xml.append("<deviceType>urn:schemas-upnp-org:device:DimmableLight:1</deviceType>");
	xml.append("<friendlyName>UPnP Sample Dimmable Light ver.1</friendlyName>");
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
	xml.append("<serviceType>" + svc_sp1 + "</serviceType>");
	xml.append("<serviceId>urn:upnp-org:serviceId:SwitchPower.1</serviceId>");
	xml.append("<SCPDURL>");
	xml.append("/scpd.xml/" + uuid + "::" + svc_sp1);
	xml.append("</SCPDURL>");
	xml.append("<controlURL>/control/" + uuid + "::" + svc_sp1);
	xml.append("</controlURL>");
	xml.append("<eventSubURL>/event/" + uuid + "::" + svc_sp1);
	xml.append("</eventSubURL>");
	xml.append("</service>");
	xml.append("<service>");
	xml.append("<serviceType>" + svc_d1 + "</serviceType>");
	xml.append("<serviceId>urn:upnp-org:serviceId:Dimming.1</serviceId>");
	xml.append("<SCPDURL>");
	xml.append("/scpd.xml/" + uuid + "::" + svc_d1);
	xml.append("</SCPDURL>");
	xml.append("<controlURL>/control/" + uuid + "::" + svc_d1);
	xml.append("</controlURL>");
	xml.append("<eventSubURL>/event/" + uuid + "::" + svc_d1);
	xml.append("</eventSubURL>");
	xml.append("</service>");
	xml.append("</serviceList>");
	xml.append("</device>");
	xml.append("</root>");

	return xml;
}

/**
 * @brief 
 */
string scpd_sp1() {

	string xml = "<scpd  xmlns=\"urn:schemas-upnp-org:service-1-0\">"
		"<specVersion>"
		"<major>1</major>"
		"<minor>0</minor>"
		"</specVersion>"
		"<actionList>"
		"<action>"
		"<name>SetTarget</name>"
		"<argumentList>"
        "<argument>"
		"<name>newTargetValue</name>"
		"<direction>in</direction>"
		"<relatedStateVariable>Target</relatedStateVariable>"
        "</argument>"
		"</argumentList>"
		"</action>"
		"<action>"
		"<name>GetTarget</name>"
		"<argumentList>"
        "<argument>"
		"<name>RetTargetValue</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>Target</relatedStateVariable>"
        "</argument>"
		"</argumentList>"
		"</action>"
		"<action>"
		"<name>GetStatus</name>"
		"<argumentList>"
        "<argument>"
		"<name>ResultStatus</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>Status</relatedStateVariable>"
        "</argument>"
		"</argumentList>"
		"</action>"
		"</actionList>"
		"<serviceStateTable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>Target</name>"
		"<dataType>boolean</dataType>"
		"<defaultValue>0</defaultValue>"
		"</stateVariable>"
		"<stateVariable>"
		"<name>Status</name>"
		"<dataType>boolean</dataType>"
		"<defaultValue>0</defaultValue>"
		"</stateVariable>"
		"</serviceStateTable>"
		"</scpd>";

	return xml;
}

string scpd_d1() {
	string xml = "<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">"
		"<specVersion>"
		"<major>1</major>"
		"<minor>0</minor>"
		"</specVersion>"
		"<actionList>"
		"<action>"
		"<name>SetLoadLevelTarget</name>"
		"<argumentList>"
        "<argument>"
		"<name>newLoadlevelTarget</name>"
		"<direction>in</direction>"
		"<relatedStateVariable>LoadLevelTarget</relatedStateVariable>"
        "</argument>"
		"</argumentList>"
		"</action>"
		"<action>"
		"<name>GetLoadLevelTarget</name>"
		"<argumentList>"
        "<argument>"
		"<name>GetLoadlevelTarget</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>LoadLevelTarget</relatedStateVariable>"
        "</argument>"
		"</argumentList>"
		"</action>"
		"<action>"
		"<name>GetLoadLevelStatus</name>"
		"<argumentList>"
        "<argument>"
		"<name>retLoadlevelStatus</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>LoadLevelStatus</relatedStateVariable>"
        "</argument>"
		"</argumentList>"
		"</action>"
		"</argumentList>"
		"</action>"
		"</actionList>"
		"<serviceStateTable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>LoadLevelTarget</name>"
		"<dataType>ui1</dataType>"
		"<defaultValue>0</defaultValue>"
		"<allowedValueRange>"
        "<minimum>0</minimum>"
        "<maximum>100</maximum>"
		"</allowedValueRange>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"yes\">"
		"<name>LoadLevelStatus</name>"
		"<dataType>ui1</dataType>"
		"<defaultValue>100</defaultValue>"
		"<allowedValueRange>"
        "<minimum>0</minimum>"
        "<maximum>100</maximum>"
		"</allowedValueRange>"
		"</stateVariable>"
		"</serviceStateTable>"
		"</scpd>";
	return xml;
}

/**
 * @brief 
 */
class MyActionRequestHandler : public UPnPActionRequestHandler {
private:
public:
    MyActionRequestHandler() { /**/ }
    virtual ~MyActionRequestHandler() { /**/ }

	virtual bool handleActionRequest(UPnPActionRequest & request, UPnPActionResponse & response) {

		cout << "** Action requst **" << endl;
		cout << " - service type : " << request.serviceType() << endl;
		cout << " - action name : " << request.actionName() << endl;
		
		if (request.actionName() == "GetStatus") {
			response["ResultStatus"] = s_lightOn ? "1" : "0";
		} else if (request.actionName() == "GetTarget") {
			response["RetTargetValue"] = s_lightOn ? "1" : "0";
		} else if (request.actionName() == "SetTarget") {
			s_lightOn = (request["newTargetValue"] == "1");
		} else if (request.actionName() == "SetLoadLevelTarget") {
			s_level = Text::toInt(request["newLoadlevelTarget"]);
		} else if (request.actionName() == "GetLoadLevelTarget") {
			response["GetLoadlevelTarget"] = Text::toString(s_level);
		} else if (request.actionName() == "GetLoadLevelStatus") {
			response["retLoadlevelStatus"] = Text::toString(s_level);
		} else {
			// unknown request
			return false;
		}

		return true;
	}
};

/**
 * @brief 
 */
static void s_set_device(UPnPServer & server, const string & uuid) {
	
	string svc_sp1 = "urn:schemas-upnp-org:service:SwitchPower:1";
	string svc_d1 = "urn:schemas-upnp-org:service:Dimming:1";
	UPnPResourceManager::properties()["/device.xml"] = dd(uuid);
	UPnPResourceManager::properties()["/scpd.xml/" + uuid + "::" + svc_sp1] = scpd_sp1();
	UPnPResourceManager::properties()["/scpd.xml/" + uuid + "::" + svc_d1] = scpd_d1();

	server.registerDeviceProfile(uuid, Url("prop:///device.xml"));

	LinkedStringMap props_sp1;
	props_sp1["RetTargetValue"] = "0";
	server.setProperties(uuid, svc_sp1, props_sp1);

	LinkedStringMap props_d1;
	props_d1["LoadLevelStatus"] = "100";
	server.setProperties(uuid, svc_d1, props_d1);
}

/**
 * @brief 
 */
class OutdatedListener : public OnSubscriptionOutdatedListener {
public:
	OutdatedListener() {}
	virtual ~OutdatedListener() {}
	virtual void onSessionOutdated(UPnPEventSubscriptionSession & session) {
		cout << "session outdated / " << session.sid() << endl;
	}
};

class PrintDebugInfo : public OnDebugInfoListener {
private:
	FileStream & stream;
public:
	PrintDebugInfo(FileStream & stream) : stream(stream) {
	}
	virtual ~PrintDebugInfo() {
	}
	virtual void onDebugInfo(const UPnPDebugInfo & info) {
		stream.writeline("[" + Date::format(Date::now(), "%Y-%c-%d %H:%i:%s.%f") + "] - " +
						 info.const_tag());
		stream.writeline(info.const_packet());
	}
};

/**
 * @brief 
 */
int main(int argc, char * args[]) {

	Arguments arguments = ArgumentParser::parse(argc, args);
	FileStream out;

	// UuidGeneratorVersion1 gen;
	// string uuid = gen.generate();
	string uuid = "e399855c-7ecb-1fff-8000-000000000000";
	
	UPnPServer server(UPnPServer::Config(9001));
	if (arguments.varAsBoolean("debug", false)) {
		out = FileStream("./.server.log", "wb");
		AutoRef<UPnPDebug> debug(new UPnPDebug);
		debug->setOnDebugInfoListener(AutoRef<OnDebugInfoListener>(new PrintDebugInfo(out)));
		server.setDebug(debug);
	}
	
	server.startAsync();
	s_set_device(server, uuid);
	server.setActionRequestHandler(AutoRef<UPnPActionRequestHandler>(new MyActionRequestHandler));
	server.getPropertyManager().
		setOnSubscriptionOutdatedListener(AutoRef<OnSubscriptionOutdatedListener>(new OutdatedListener));

	cout << "uuid: " << uuid << endl;

	while (1) {
		string cmd;
		if ((cmd = readline()).size() > 0) {
			if (cmd == "q" || cmd == "quit") {
				break;
			}

			vector<string> tokens = Text::split(cmd, " ");

			if (tokens.size() == 0) {
				continue;
			}

			if (tokens[0] == "alive") {
				cout << " * alive : " << uuid << endl;
				server.setEnableDevice(uuid, true);
			} else if (tokens[0] == "byebye") {
				cout << " * byebye : " << uuid << endl;
				server.setEnableDevice(uuid, false);
			} else if (tokens[0] == "list") {
				vector<AutoRef<UPnPDeviceProfileSession> > vec = server.getProfileManager().sessionList();
				for (size_t i = 0; i < vec.size(); i++) {
					UPnPDeviceProfile & profile = vec[i]->profile();
					cout << "[" << i << "] " << profile.uuid() << " ; " << (profile.deviceTypes().size() > 0 ? profile.deviceTypes()[0] : "") <<
						 " / " << (vec[i]->isEnabled() ? "enabled" : "disabled") << endl;
				}
			} else if (tokens[0] == "set-props") {
				LinkedStringMap props_sp1;
				props_sp1["RetTargetValue"] = s_lightOn ? "1" : "0";
				server.setProperties(uuid, "urn:schemas-upnp-org:service:SwitchPower:1", props_sp1);

				LinkedStringMap props_d1;
				props_d1["LoadLevelStatus"] = Text::toString(s_level);
				server.setProperties(uuid, "urn:schemas-upnp-org:service:Dimming:1", props_d1);
			} else if (tokens[0] == "load") {
				if (tokens.size() < 2) {
					continue;
				}
				string uri = tokens[1];
				// TODO: implement dynamic load
			} else if (tokens[0] == "unload") {
				if (tokens.size() < 2) {
					continue;
				}
				string uuid = tokens[1];
				// TODO: implement dynamic unload
			}
		}
	}

	server.stop();

	out.close();
    
    return 0;
}
