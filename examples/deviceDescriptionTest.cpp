#include <libupnp-tools/UPnPDeviceXmlWriter.hpp>
#include <libupnp-tools/XmlDocumentPrinter.hpp>
#include <libupnp-tools/UPnPService.hpp>
#include <iostream>

using namespace std;
using namespace UPNP;
using namespace XML;


static void test_dd() {
    UPnPDevice device;
    
    device["deviceType"] = "urn:schemas-upnp-org:device:MediaServer:1";
    device["friendlyName"] = "Plex Media Server: xxx";
    device["manufacturer"] = "Plex, Inc.";
    device["manufacturerURL"] = "http://www.plexapp.com/";
    device["modelDescription"] = "Plex Media Server";
    device["modelName"] = "Plex Media Server";
    device["modelURL"] = "http://www.plexapp.com/";
    device["modelNumber"] = "0.9.12.8";
    device["serialNumber"] = "";
    device["UDN"] = "uuid:804acd56-ba2e-35e9-8619-9bbd28b27b0e";
    device["dlna:X_DLNADOC"] = "DMS-1.50";
    
    UPnPService service;
    service["serviceType"] = "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1";
    service["serviceId"] = "urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar";
    service["SCPDURL"] = "/X_MS_MediaReceiverRegistrar/804acd56-ba2e-35e9-8619-9bbd28b27b0e/scpd.xml";
    service["controlURL"] = "/X_MS_MediaReceiverRegistrar/804acd56-ba2e-35e9-8619-9bbd28b27b0e/control.xml";
    service["eventSubURL"] = "/X_MS_MediaReceiverRegistrar/804acd56-ba2e-35e9-8619-9bbd28b27b0e/event.xml";
    device.addService(service);
    
    service.clear();
    service["serviceType"] = "urn:xxxxxxxxxxxxx:1";
    service["serviceId"] = "urn:xxxxxxxxxxxxx";
    service["SCPDURL"] = "/xxxxxxxxxxxxx/scpd.xml";
    service["controlURL"] = "/xxxxxxxxxxxxx/control.xml";
    service["eventSubURL"] = "/xxxxxxxxxxxxx/event.xml";
    device.addService(service);
    
    UPnPDevice embed;
    embed["UPC"] = "UPNPD";
    device.addEmbeddedDevice(embed);
    
    XmlDocument doc = UPnPDeviceXmlWriter::makeDeviceDescriptionXmlDocument(device);
    
    XmlPrinter printer;
    printer.setFormatted(true);
    
    string dump = printer.printDocument(doc);
    cout << dump << endl;
}

static void test_scpd() {
    Scpd scpd;
    
    vector<UPnPAction> actions;
    vector<UPnPStateVariable> stateVariables;
    
    UPnPStateVariable stateVariable;
    stateVariable.setName("A_ARG_TYPE_ObjectID");
    stateVariable["sendEvents"] = "no";
    stateVariable.setDataType("string");
    
    stateVariables.push_back(stateVariable);
    
    scpd.setStateVariables(stateVariables);
    
    UPnPAction action;
    action.setName("Browse");
    
    vector<UPnPActionArgument> arguments;
    UPnPActionArgument argument;
    argument.setName("ObjectID");
    argument.setDirection("in");
    argument.setStateVariable(stateVariable);
    arguments.push_back(argument);
    
    action.setArguments(arguments);
    
    actions.push_back(action);
    
    scpd.setActions(actions);
    
    XmlDocument doc = ScpdXmlWriter::makeScpdXmlDocument(scpd);
    
    XmlPrinter printer;
    printer.setFormatted(true);
    
    string dump = printer.printDocument(doc);
    cout << dump << endl;
}

int main(int argc, char * args[]) {
    
//    test_dd();
    test_scpd();
    
    return 0;
}
