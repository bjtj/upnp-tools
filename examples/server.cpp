#include <iostream>
#include <string>
#include <map>
#include <liboslayer/Text.hpp>
#include <libupnp-tools/UPnPServer.hpp>

using namespace std;
using namespace UTIL;
using namespace UPNP;

Scpd makeScpd() {

	Scpd scpd;

	vector<UPnPStateVariable> stateVariables;

	UPnPStateVariable stateVariableSourceProtocolInfo;
	stateVariableSourceProtocolInfo.setName("SourceProtocolInfo");
	stateVariableSourceProtocolInfo.setDataType("string");

	stateVariables.push_back(stateVariableSourceProtocolInfo);

	UPnPStateVariable stateVariableSinkProtocolInfo;
	stateVariableSinkProtocolInfo.setName("SinkProtocolInfo");
	stateVariableSinkProtocolInfo.setDataType("string");

	stateVariables.push_back(stateVariableSinkProtocolInfo);
	
	vector<UPnPAction> actions;

	UPnPAction action;
	action.setName("GetProtocolInfo");

	vector<UPnPActionArgument> arguments;

	UPnPActionArgument argument;
	argument.setName("Source");
	argument.setDirection("out");
	argument.setStateVariable(stateVariableSourceProtocolInfo);

	arguments.push_back(argument);
          
	argument.setName("Sink");
	argument.setDirection("out");
	argument.setStateVariable(stateVariableSinkProtocolInfo);

	arguments.push_back(argument);

	action.setArguments(arguments);

	actions.push_back(action);

	scpd.setActions(actions);
	scpd.setStateVariables(stateVariables);

	return scpd;
}

UPnPDevice makeDevice(UrlSerializer & serializer) {

	string udn = "uuid:fc4ec57e-b051-11db-88f8-006008abcdef";

	UPnPDevice device;
    device.setUdn(udn);
    device.setFriendlyName("Dummy Device");
	device["deviceType"] = "urn:schemas-upnp-org:device:MediaServer:1";
    device["manufacturer"] = "Plex, Inc.";
    device["manufacturerURL"] = "http://www.plexapp.com/";
    device["modelDescription"] = "Plex Media Server";
    device["modelName"] = "Plex Media Server";
    device["modelURL"] = "http://www.plexapp.com/";
    device["modelNumber"] = "0.9.12.8";
    device["serialNumber"] = "";
    device["dlna:X_DLNADOC"] = "DMS-1.50";
	device["dlna:X_DLNADOC"].setProperty("xmlns:dlna", "urn:schemas-dlna-org:device-1-0");

	UPnPService service;

	service.clear();
    service["serviceType"] = "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1";
    service["serviceId"] = "urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar";
	service["SCPDURL"] = serializer.makeUrlPath(udn, "/a/scpd.xml");
    service["controlURL"] = serializer.makeUrlPath(udn, "/a/control.xml");
    service["eventSubURL"] = serializer.makeUrlPath(udn, "/a/event.xml");

	device.addService(service);
     
	service.clear();
    service["serviceType"] = "urn:schemas-upnp-org:service:ContentDirectory:1";
    service["serviceId"] = "urn:upnp-org:serviceId:ContentDirectory";
    service["SCPDURL"] = serializer.makeUrlPath(udn, "/b/scpd.xml");
    service["controlURL"] = serializer.makeUrlPath(udn, "/b/control.xml");
    service["eventSubURL"] = serializer.makeUrlPath(udn, "/b/event.xml");

	device.addService(service);
     
	service.clear();
    service["serviceType"] = "urn:schemas-upnp-org:service:ConnectionManager:1";
    service["serviceId"] = "urn:upnp-org:serviceId:ConnectionManager";
    service["SCPDURL"] = serializer.makeUrlPath(udn, "/c/scpd.xml");
    service["controlURL"] = serializer.makeUrlPath(udn, "/c/control.xml");
    service["eventSubURL"] = serializer.makeUrlPath(udn, "/c/event.xml");

	service.setScpd(makeScpd());

	device.addService(service);

	return device;
}

size_t readline(char * buffer, size_t max) {
    fgets(buffer, (int)max - 1, stdin);
    buffer[strlen(buffer) - 1] = 0;
    return strlen(buffer);
}

class ActionHandler : public UPnPActionRequestHandler {
private:
public:
	ActionHandler() {
	}
	virtual ~ActionHandler() {
	}
	void onActionRequest(const UPnPActionRequest & request, const UPnPActionResponse & response) {
	}
};

int main(int argc, char * args[]) {
    
    bool done = false;
    char buffer[1024] = {0,};
    UPnPServer server(8083);

	ActionHandler actionHandler;
    
	server.setActionRequestHandler(&actionHandler);
	UPnPDevice device = makeDevice(server.getUrlSerializer());

    server.registerDevice(device);
    
    server.startAsync();
    
    while (!done) {
        readline(buffer, sizeof(buffer));
        if (!strcmp(buffer, "q")) {
            done = true;
            break;
        }
        if (!strcmp(buffer, "a")) {
            server.announceDevice(server.getDevice("uuid:fc4ec57e-b051-11db-88f8-006008abcdef"));
        }
    }
    
    server.stop();
    
    return 0;
}