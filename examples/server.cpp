#include <iostream>
#include <string>
#include <map>
#include <liboslayer/Text.hpp>
#include <libupnp-tools/UPnPServer.hpp>
#include <libupnp-tools/UPnPDeviceMaker.hpp>
#include <libupnp-tools/UPnPServiceMaker.hpp>
#include <libupnp-tools/XmlDomParser.hpp>

using namespace std;
using namespace UTIL;
using namespace UPNP;
using namespace XML;

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


UPnPDevice makeDeviceFromXml(UrlSerializer & serializer) {
    
    string dd =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<root xmlns=\"urn:schemas-upnp-org:device-1-0\" xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">"
    "<specVersion>"
    "<major>1</major>"
    "<minor>0</minor>"
    "</specVersion>"
    "<device>"
    "<deviceType>urn:schemas-upnp-org:device:MediaServer:1</deviceType>"
    "<friendlyName>Plex Media Server: tjjang-Latitude-E6510</friendlyName>"
    "<manufacturer>Plex, Inc.</manufacturer>"
    "<manufacturerURL>http://www.plexapp.com/</manufacturerURL>"
    "<modelDescription>Plex Media Server</modelDescription>"
    "<modelName>Plex Media Server</modelName>"
    "<modelURL>http://www.plexapp.com/</modelURL>"
    "<modelNumber>0.9.12.8</modelNumber>"
    "<serialNumber/>"
    "<UDN>${UDN}</UDN>"
    "<dlna:X_DLNADOC xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">DMS-1.50</dlna:X_DLNADOC>"
    "<serviceList>"
    "<service>"
    "<serviceType>urn:schemas-upnp-org:service:ContentDirectory:1</serviceType>"
    "<serviceId>urn:upnp-org:serviceId:ContentDirectory</serviceId>"
    "<SCPDURL>${PREFIX}/cd/scpd.xml</SCPDURL>"
    "<controlURL>${PREFIX}cd/control.xml</controlURL>"
    "<eventSubURL>${PREFIX}cd/event.xml</eventSubURL>"
    "</service>"
    "<service>"
    "<serviceType>urn:schemas-upnp-org:service:ConnectionManager:1</serviceType>"
    "<serviceId>urn:upnp-org:serviceId:ConnectionManager</serviceId>"
    "<SCPDURL>${PREFIX}/cm/scpd.xml</SCPDURL>"
    "<controlURL>${PREFIX}cm/control.xml</controlURL>"
    "<eventSubURL>${PREFIX}cm/event.xml</eventSubURL>"
    "</service>"
    "</serviceList>"
    "</device>"
    "</root>";
    
   	string udn = "uuid:fc4ec57e-b051-11db-88f8-006008abcdef";
    dd = Text::replaceAll(dd, "${UDN}", udn);
    dd = Text::replaceAll(dd, "${PREFIX}", serializer.makeUrlPathPrefix(udn));
    
    XmlDomParser parser;
    XmlDocument doc = parser.parse(dd);
    
    UPnPDevice device = UPnPDeviceMaker::makeDeviceFromDeviceDescription("", doc);
    
    
    string cd = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">"
    "<specVersion>"
    "<major>1</major>"
    "<minor>0</minor>"
    "</specVersion>"
    "<actionList>"
    "<action>"
    "<name>Browse</name>"
    "<argumentList>"
    "<argument>"
	"<name>ObjectID</name>"
	"<direction>in</direction>"
	"<relatedStateVariable>A_ARG_TYPE_ObjectID</relatedStateVariable>"
    "</argument>"
    "<argument>"
	"<name>BrowseFlag</name>"
	"<direction>in</direction>"
	"<relatedStateVariable>A_ARG_TYPE_BrowseFlag</relatedStateVariable>"
    "</argument>"
    "<argument>"
	"<name>Filter</name>"
	"<direction>in</direction>"
	"<relatedStateVariable>A_ARG_TYPE_Filter</relatedStateVariable>"
    "</argument>"
    "<argument>"
	"<name>StartingIndex</name>"
	"<direction>in</direction>"
	"<relatedStateVariable>A_ARG_TYPE_Index</relatedStateVariable>"
    "</argument>"
    "<argument>"
	"<name>RequestedCount</name>"
	"<direction>in</direction>"
	"<relatedStateVariable>A_ARG_TYPE_Count</relatedStateVariable>"
    "</argument>"
    "<argument>"
	"<name>SortCriteria</name>"
	"<direction>in</direction>"
	"<relatedStateVariable>A_ARG_TYPE_SortCriteria</relatedStateVariable>"
    "</argument>"
    "<argument>"
	"<name>Result</name>"
	"<direction>out</direction>"
	"<relatedStateVariable>A_ARG_TYPE_Result</relatedStateVariable>"
    "</argument>"
    "<argument>"
	"<name>NumberReturned</name>"
	"<direction>out</direction>"
	"<relatedStateVariable>A_ARG_TYPE_Count</relatedStateVariable>"
    "</argument>"
    "<argument>"
	"<name>TotalMatches</name>"
	"<direction>out</direction>"
	"<relatedStateVariable>A_ARG_TYPE_Count</relatedStateVariable>"
    "</argument>"
    "<argument>"
	"<name>UpdateID</name>"
	"<direction>out</direction>"
	"<relatedStateVariable>A_ARG_TYPE_UpdateID</relatedStateVariable>"
    "</argument>"
    "</argumentList>"
    "</action>"
    "<action>"
    "<name>GetSortCapabilities</name>"
    "<argumentList>"
	"<argument>"
    "<name>SortCaps</name>"
    "<direction>out</direction>"
    "<relatedStateVariable>SortCapabilities</relatedStateVariable>"
	"</argument>"
    "</argumentList>"
    "</action>"
    "<action>"
    "<name>GetSystemUpdateID</name>"
    "<argumentList>"
	"<argument>"
    "<name>Id</name>"
    "<direction>out</direction>"
    "<relatedStateVariable>SystemUpdateID</relatedStateVariable>"
	"</argument>"
    "</argumentList>"
    "</action>"
    "<action>"
    "<name>GetSearchCapabilities</name>"
    "<argumentList>"
	"<argument>"
    "<name>SearchCaps</name>"
    "<direction>out</direction>"
    "<relatedStateVariable>SearchCapabilities</relatedStateVariable>"
	"</argument>"
    "</argumentList>"
    "</action>"
    "</actionList>"
    "<serviceStateTable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>A_ARG_TYPE_BrowseFlag</name>"
    "<dataType>string</dataType>"
    "<allowedValueList>"
	"<allowedValue>BrowseMetadata</allowedValue>"
	"<allowedValue>BrowseDirectChildren</allowedValue>"
    "</allowedValueList>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"yes\">"
    "<name>ContainerUpdateIDs</name>"
    "<dataType>string</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"yes\">"
    "<name>SystemUpdateID</name>"
    "<dataType>ui4</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>A_ARG_TYPE_Count</name>"
    "<dataType>ui4</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>A_ARG_TYPE_SortCriteria</name>"
    "<dataType>string</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>SortCapabilities</name>"
    "<dataType>string</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>A_ARG_TYPE_Index</name>"
    "<dataType>ui4</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>A_ARG_TYPE_ObjectID</name>"
    "<dataType>string</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>A_ARG_TYPE_UpdateID</name>"
    "<dataType>ui4</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>A_ARG_TYPE_Result</name>"
    "<dataType>string</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>SearchCapabilities</name>"
    "<dataType>string</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>A_ARG_TYPE_Filter</name>"
    "<dataType>string</dataType>"
    "</stateVariable>"
    "</serviceStateTable>"
    "</scpd>";
    
    doc = parser.parse(cd);
    Scpd scpd = UPnPServiceMaker::makeScpdFromXmlDocument("urn:schemas-upnp-org:service:ContentDirectory:1", doc);
    device.getServiceWithServiceType("urn:schemas-upnp-org:service:ContentDirectory:1").setScpd(scpd);
    
    string cm =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">"
    "<specVersion>\""
    "<major>1</major>\""
    "<minor>0</minor>\""
    "</specVersion>\""
    "<actionList>"
    "<action>"
    "<name>GetCurrentConnectionInfo</name>"
    "<argumentList>"
    "<argument>"
    "<name>ConnectionID</name>"
    "<direction>in</direction>"
    "<relatedStateVariable>A_ARG_TYPE_ConnectionID</relatedStateVariable>"
    "</argument>"
    "<argument>"
    "<name>RcsID</name>"
    "<direction>out</direction>"
    "<relatedStateVariable>A_ARG_TYPE_RcsID</relatedStateVariable>"
    "</argument>"
    "<argument>"
    "<name>AVTransportID</name>"
    "<direction>out</direction>"
    "<relatedStateVariable>A_ARG_TYPE_AVTransportID</relatedStateVariable>"
    "</argument>"
    "<argument>"
    "<name>ProtocolInfo</name>"
    "<direction>out</direction>"
    "<relatedStateVariable>A_ARG_TYPE_ProtocolInfo</relatedStateVariable>"
    "</argument>"
    "<argument>"
    "<name>PeerConnectionManager</name>"
    "<direction>out</direction>"
    "<relatedStateVariable>A_ARG_TYPE_ConnectionManager</relatedStateVariable>"
    "</argument>"
    "<argument>"
    "<name>PeerConnectionID</name>"
    "<direction>out</direction>"
    "<relatedStateVariable>A_ARG_TYPE_ConnectionID</relatedStateVariable>"
    "</argument>"
    "<argument>"
    "<name>Direction</name>"
    "<direction>out</direction>"
    "<relatedStateVariable>A_ARG_TYPE_Direction</relatedStateVariable>"
    "</argument>"
    "<argument>"
    "<name>Status</name>"
    "<direction>out</direction>"
    "<relatedStateVariable>A_ARG_TYPE_ConnectionStatus</relatedStateVariable>"
    "</argument>"
    "</argumentList>"
    "</action>"
    "<action>"
    "<name>GetProtocolInfo</name>"
    "<argumentList>"
    "<argument>"
    "<name>Source</name>"
    "<direction>out</direction>"
    "<relatedStateVariable>SourceProtocolInfo</relatedStateVariable>"
    "</argument>"
    "<argument>"
    "<name>Sink</name>"
    "<direction>out</direction>"
    "<relatedStateVariable>SinkProtocolInfo</relatedStateVariable>"
    "</argument>"
    "</argumentList>"
    "</action>"
    "<action>"
    "<name>GetCurrentConnectionIDs</name>"
    "<argumentList>"
    "<argument>"
    "<name>ConnectionIDs</name>"
    "<direction>out</direction>"
    "<relatedStateVariable>CurrentConnectionIDs</relatedStateVariable>"
    "</argument>"
    "</argumentList>"
    "</action>"
    "</actionList>"
    "<serviceStateTable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>A_ARG_TYPE_ProtocolInfo</name>"
    "<dataType>string</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>A_ARG_TYPE_ConnectionStatus</name>"
    "<dataType>string</dataType>"
    "<allowedValueList>"
    "<allowedValue>OK</allowedValue>"
    "<allowedValue>ContentFormatMismatch</allowedValue>"
    "<allowedValue>InsufficientBandwidth</allowedValue>"
    "<allowedValue>UnreliableChannel</allowedValue>"
    "<allowedValue>Unknown</allowedValue>"
    "</allowedValueList>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>A_ARG_TYPE_AVTransportID</name>"
    "<dataType>i4</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>A_ARG_TYPE_RcsID</name>"
    "<dataType>i4</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>A_ARG_TYPE_ConnectionID</name>"
    "<dataType>i4</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>A_ARG_TYPE_ConnectionManager</name>"
    "<dataType>string</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"yes\">"
    "<name>SourceProtocolInfo</name>"
    "<dataType>string</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"yes\">"
    "<name>SinkProtocolInfo</name>"
    "<dataType>string</dataType>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"no\">"
    "<name>A_ARG_TYPE_Direction</name>"
    "<dataType>string</dataType>"
    "<allowedValueList>"
    "<allowedValue>Input</allowedValue>"
    "<allowedValue>Output</allowedValue>"
    "</allowedValueList>"
    "</stateVariable>"
    "<stateVariable sendEvents=\"yes\">"
    "<name>CurrentConnectionIDs</name>"
    "<dataType>string</dataType>"
    "</stateVariable>"
    "</serviceStateTable>"
    "</scpd>";
    
    doc = parser.parse(cm);
    scpd = UPnPServiceMaker::makeScpdFromXmlDocument("urn:schemas-upnp-org:service:ConnectionManager:1", doc);
    device.getServiceWithServiceType("urn:schemas-upnp-org:service:ConnectionManager:1").setScpd(scpd);
    
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
	void onActionRequest(const UPnPActionRequest & request, UPnPActionResponse & response) {
        cout << "onActionRequest" << endl;
        response["Source"] = "source";
        response["Sink"] = "<&test xml decode&>";
        
        response.setResult(UPnPActionResult(true, 200, "OK"));
	}
};

int main(int argc, char * args[]) {
    
    bool done = false;
    char buffer[1024] = {0,};
    UPnPServer server(8083);

	ActionHandler actionHandler;
    server.setActionRequestHandler(&actionHandler);

//	UPnPDevice device = makeDevice(server.getUrlSerializer());
    UPnPDevice device = makeDeviceFromXml(server.getUrlSerializer());
    
    server.startAsync();
    
    while (!done) {
        readline(buffer, sizeof(buffer));
        if (!strcmp(buffer, "q")) {
            done = true;
            break;
        }
        if (!strcmp(buffer, "a")) {
            server.registerDevice(device);
        }
        if (!strcmp(buffer, "b")) {
            server.unregisterDevice(device.getUdn());
        }
    }
    
    server.stop();
    
    return 0;
}