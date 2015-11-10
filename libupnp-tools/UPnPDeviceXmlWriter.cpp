#include "UPnPDeviceXmlWriter.hpp"
#include <liboslayer/Text.hpp>

namespace UPNP {

    using namespace std;
    using namespace UTIL;
    using namespace XML;
    
    UPnPDeviceXmlWriter::UPnPDeviceXmlWriter() {
    }
    UPnPDeviceXmlWriter::~UPnPDeviceXmlWriter() {
    }
    
    XmlDocument UPnPDeviceXmlWriter::makeDeviceDescriptionXmlDocument(const UPnPDevice & device) {
        XmlNode rootNode;
        rootNode.setTagName("root");
        rootNode.setAttribute("xmlns", "urn:schemas-upnp-org:device-1-0");
        rootNode.setAttribute("xmlns:dlna", "urn:schemas-dlna-org:device-1-0");
        
        XmlNode specVersionNode;
        specVersionNode.setTagName("specVersion");
        specVersionNode.addNode(nameValueXml("major", "1"));
        specVersionNode.addNode(nameValueXml("minor", "0"));
        rootNode.addNode(specVersionNode);
        
        rootNode.addNode(makeDeviceXmlNode(device));
        
        XmlDocument doc;
        doc.setPrologue("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        doc.setRootNode(rootNode);
        
        return doc;
    }
    XmlNode UPnPDeviceXmlWriter::makeDeviceXmlNode(const UPnPDevice & device) {
        XmlNode deviceNode;
        deviceNode.setTagName("device");
        const LinkedStringProperties props = device.getProperties();
		for (size_t i = 0; i < props.size(); i++) {
			const NameProperty & prop = props[i];
			XmlNode node = nameValueXml(prop.getName(), prop.getValue());
			const LinkedStringMap attrs = prop.getProperties();
			for (size_t j = 0; j < attrs.size(); j++) {
				const NameValue & attr = attrs[j];
				node.setAttribute(attr.getName(), attr.getValue());
			}
            deviceNode.addNode(node);
        }
        
        if (device.getServices().size() > 0) {
            XmlNode servicesNode;
            servicesNode.setTagName("serviceList");
            const vector<UPnPService> & services = device.getServices();
            for (vector<UPnPService>::const_iterator iter = services.begin(); iter != services.end(); iter++) {
                servicesNode.addNode(makeServiceXmlNode(*iter));
            }
            deviceNode.addNode(servicesNode);
        }
        
        if (device.getEmbeddedDevices().size() > 0) {
            XmlNode devicesNode;
            devicesNode.setTagName("deviceList");
            const vector<UPnPDevice> & embeds = device.getEmbeddedDevices();
            for (vector<UPnPDevice>::const_iterator iter = embeds.begin(); iter != embeds.end(); iter++) {
                devicesNode.addNode(makeDeviceXmlNode(*iter));
            }
            deviceNode.addNode(devicesNode);
        }
        
        return deviceNode;
    }
    XmlNode UPnPDeviceXmlWriter::makeServiceXmlNode(const UPnPService & service) {
        const LinkedStringMap props = service.getProperties();
        XmlNode serviceNode;
        serviceNode.setTagName("service");
        
        for (size_t i = 0; i < props.size(); i++) {
            const NameValue & nv = props[i];
            serviceNode.addNode(nameValueXml(nv.getName(), nv.getValue()));
        }
        
        return serviceNode;
    }
    
    XmlNode UPnPDeviceXmlWriter::nameValueXml(const string & name, const string & value) {
        XmlNode node;
        node.setTagName(name);
        XmlNode contentNode;
        contentNode.setData(value);
        node.addNode(contentNode);
        return node;
    }
    
    /**
     *
     */
    
    ScpdXmlWriter::ScpdXmlWriter() {
        
    }
    ScpdXmlWriter::~ScpdXmlWriter() {
        
    }
    
    XmlDocument ScpdXmlWriter::makeScpdXmlDocument(const Scpd & scpd) {
        XmlNode rootNode;
        rootNode.setTagName("scpd");
        rootNode.setAttribute("xmlns", "urn:schemas-upnp-org:service-1-0");
        
        XmlNode specVersionNode;
        specVersionNode.setTagName("specVersion");
        specVersionNode.addNode(nameValueXml("major", "1"));
        specVersionNode.addNode(nameValueXml("minor", "0"));
        rootNode.addNode(specVersionNode);
        
        XmlNode actionListNode;
        actionListNode.setTagName("actionList");
        const vector<UPnPAction> & actions = scpd.getActions();
        for (size_t i = 0; i < actions.size(); i++) {
            actionListNode.addNode(makeActionXmlNode(actions[i]));
        }
        rootNode.addNode(actionListNode);
        
        XmlNode serviceStateTableNode;
        serviceStateTableNode.setTagName("serviceStateTable");
        const vector<UPnPStateVariable> & stateVariables = scpd.getStateVariables();
        for (size_t i = 0; i < stateVariables.size(); i++) {
            serviceStateTableNode.addNode(makeStateVariableXmlNode(stateVariables[i]));
        }
        rootNode.addNode(serviceStateTableNode);
        
        XmlDocument doc;
        doc.setPrologue("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        doc.setRootNode(rootNode);
        
        return doc;
    }
    XmlNode ScpdXmlWriter::makeActionXmlNode(const UPnPAction & action) {
        XmlNode actionNode;
        actionNode.setTagName("action");
        actionNode.addNode(nameValueXml("name", action.getName()));
        
        XmlNode argumentListNode;
        argumentListNode.setTagName("argumentList");
        const vector<UPnPActionArgument> & arguments = action.getArguments();
        for (size_t i = 0; i < arguments.size(); i++) {
            const UPnPActionArgument & argument = arguments[i];
            XmlNode argumentNode = makeActionArgumentXmlNode(argument);
            argumentListNode.addNode(argumentNode);
        }
        actionNode.addNode(argumentListNode);
        
        return actionNode;
    }
    XmlNode ScpdXmlWriter::makeActionArgumentXmlNode(const UPnPActionArgument & argument) {
        XmlNode argumentNode;
        argumentNode.setTagName("argument");
        argumentNode.addNode(nameValueXml("name", argument.getName()));
        argumentNode.addNode(nameValueXml("direction", (argument.in() ? "in" : "out")));
        argumentNode.addNode(nameValueXml("relatedStateVariable", argument.getStateVariableName()));
        return argumentNode;
    }
    XmlNode ScpdXmlWriter::makeStateVariableXmlNode(const UPnPStateVariable & stateVariable) {
        XmlNode stateVariableNode;
        stateVariableNode.setTagName("stateVariable");
        string sendEvents = stateVariable.getProperty("sendEvents");
        stateVariableNode.setAttribute("sendEvents", Text::equalsIgnoreCase(sendEvents, "yes") ? "yes" : "no");
        stateVariableNode.addNode(nameValueXml("name", stateVariable.getName()));
        stateVariableNode.addNode(nameValueXml("dataType", stateVariable.getDataType()));
        if (stateVariable.getAllowedValueList().size() > 0) {
            XmlNode allowedValueListNode;
            allowedValueListNode.setTagName("allowedValueList");
            const vector<string> & allowedValues = stateVariable.getAllowedValueList();
            for (size_t i = 0; i < allowedValues.size(); i++) {
                string value = allowedValues[i];
                allowedValueListNode.addNode(nameValueXml("allowedValue", value));
            }
            stateVariableNode.addNode(allowedValueListNode);
        }
        return stateVariableNode;
    }
    
    XmlNode ScpdXmlWriter::nameValueXml(const string & name, const string & value) {
        XmlNode node;
        node.setTagName(name);
        XmlNode contentNode;
        contentNode.setData(value);
        node.addNode(contentNode);
        return node;
    }
}