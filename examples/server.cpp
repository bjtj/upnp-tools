#include <iostream>
#include <liboslayer/os.hpp>
#include <liboslayer/Arguments.hpp>
#include <liboslayer/FileStream.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <liboslayer/Uuid.hpp>
#include <liboslayer/Logger.hpp>
#include <libupnp-tools/UPnPModels.hpp>
#include <libupnp-tools/SSDPMsearchSender.hpp>
#include <libupnp-tools/NetworkUtil.hpp>
#include <libupnp-tools/UPnPServer.hpp>
#include <libupnp-tools/UPnPActionRequestHandler.hpp>
#include <libupnp-tools/UPnPDeviceDeserializer.hpp>
#include <libupnp-tools/UPnPResourceManager.hpp>

using namespace std;
using namespace osl;
using namespace ssdp;
using namespace http;
using namespace upnp;

static bool s_lightOn = false;
static int s_level = 100;


static string s_readline(const string & prompt);
static AutoRef<UPnPDeviceProfile> s_set_device(UPnPServer & server, const string & udn);
static void s_list_profiles(UPnPServer & server);


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
class OutdatedListener : public OnSubscriptionOutdatedListener {
public:
    OutdatedListener() {}
    virtual ~OutdatedListener() {}
    virtual void onSessionOutdated(UPnPEventSubscriptionSession & session) {
	cout << "session outdated / " << session.sid() << endl;
    }
};


/**
 * 
 */
class FileWriter : public LogWriter {
private:
    FileStream _stream;
public:
    FileWriter(const string & path) : _stream(path, "wb") {
    }
	
    virtual ~FileWriter() {
    }
	
    virtual void write(const string & str) {
	_stream.writeline(str);
    }
};


/**
 * @brief 
 */
int main(int argc, char * args[]) {

    AutoRef<LogWriter> writer(new FileWriter(".controlpoint.log"));
    LoggerFactory::instance().registerWriter("filewriter", writer);
    LoggerFactory::instance().setProfile("UPnP*", "basic", "filewriter");

    Arguments arguments = ArgumentParser::parse(argc, args);

    // UuidGeneratorVersion1 gen;
    // string uuid = gen.generate();
    string udn = "uuid:e399855c-7ecb-1fff-8000-000000000000";
	
    UPnPServer server(UPnPServer::Config(9001));

    // set device
    s_set_device(server, udn);
    server.setProperty(udn, "urn:schemas-upnp-org:service:SwitchPower:1",
		       "RetTargetValue", "0");
    server.setProperty(udn, "urn:schemas-upnp-org:service:Dimming:1",
		       "LoadLevelStatus", "100");

    // action handler
    server.setActionRequestHandler(AutoRef<UPnPActionRequestHandler>(new MyActionRequestHandler));
    server.getPropertyManager().setOnSubscriptionOutdatedListener(
	AutoRef<OnSubscriptionOutdatedListener>(new OutdatedListener));
    server.startAsync();

    cout << "UPnP Server running / udn: " << udn << endl;

    while (1) {
	string cmd;
	if ((cmd = s_readline("> ")).size() > 0) {
	    if (cmd == "quit" || cmd == "q") {
		cout << "[quit]" << endl;
		break;
	    }

	    vector<string> tokens = Text::split(cmd, " ");

	    if (tokens.size() == 0) {
		continue;
	    }

	    if (tokens[0] == "alive") {
		cout << " * alive : " << udn << endl;
		server.activateDevice(udn);
	    } else if (tokens[0] == "byebye") {
		cout << " * byebye : " << udn << endl;
		server.deactivateDevice(udn);
	    } else if (tokens[0] == "list") {
		s_list_profiles(server);
	    } else if (tokens[0] == "set-props") {
		server.setProperty(udn, "urn:schemas-upnp-org:service:SwitchPower:1",
				   "RetTargetValue", (s_lightOn ? "1" : "0"));
		server.setProperty(udn, "urn:schemas-upnp-org:service:Dimming:1",
				   "LoadLevelStatus", Text::toString(s_level));
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

    server.deactivateAllDevices();
    server.stop();

    cout << "[done]" << endl;
    
    return 0;
}


/**
 * @brief 
 */
static string s_readline(const string & prompt) {
    cout << prompt;
    FileStream fs(stdin);
    return fs.readline();
}


/**
 * @brief 
 */
static AutoRef<UPnPDeviceProfile> s_set_device(UPnPServer & server, const string & udn) {
    
#if !defined(DATA_PATH)
    string DATA_PATH = File::merge(File::getCwd(), "data");
#endif

    AutoRef<UPnPDeviceProfile> profile =
	server.registerDeviceProfile("file://" + string(DATA_PATH) + "/dimming-light.xml");

    profile->setUdn(udn);
    profile->device()->setScpdUrl("/$udn/$serviceType/scpd.xml");
    profile->device()->setControlUrl("/$udn/$serviceType/control.xml");
    profile->device()->setEventSubUrl("/$udn/$serviceType/event.xml");

    return profile;
}


/**
 * @brief
 */
static void s_list_profiles(UPnPServer & server) {
    vector< AutoRef<UPnPDeviceProfile> > vec = server.getProfileManager().profiles();
    for (size_t i = 0; i < vec.size(); i++) {
	AutoRef<UPnPDeviceProfile> profile = vec[i];
	cout << "[" << i << "] " <<
	    profile->udn() <<
	    " ; " <<
	    (profile->deviceTypes().size() > 0 ? profile->deviceTypes()[0] : "") <<
	    " / " <<
	    (vec[i]->enabled() ? "enabled" : "disabled") << endl;
    }
}
