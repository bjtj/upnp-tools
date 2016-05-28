#include "UPnPDeviceSerializer.hpp"
#include "XmlUtils.hpp"
#include <liboslayer/Text.hpp>

namespace UPNP {

	using namespace std;
	using namespace UTIL;

	string UPnPDeviceSerializer::FIRSTLINE = "<?xml version=\"1.0\"?>";
	string UPnPDeviceSerializer::NEWLINE = "\r\n";
	
	UPnPDeviceSerializer::UPnPDeviceSerializer() {
	}
	UPnPDeviceSerializer::~UPnPDeviceSerializer() {
	}

	string UPnPDeviceSerializer::serializeDeviceDescription(UPnPDevice & device) {
		string ret = FIRSTLINE;
		ret.append(NEWLINE);
		ret.append("<root xmlns=\"urn:schemas-upnp-org:device-1-0\">");
		ret.append(NEWLINE);
		ret.append("<specVersion>");
		ret.append(NEWLINE);
		ret.append("<major>1</major>");
		ret.append(NEWLINE);
		ret.append("<minor>0</minor>");
		ret.append(NEWLINE);
		ret.append("</specVersion>");
		ret.append(NEWLINE);

		ret.append(serializeDevice(device));
		ret.append(NEWLINE);
			
		ret.append("</root>");
		ret.append(NEWLINE);
		
		return ret;
	}

	string UPnPDeviceSerializer::serializeDevice(UPnPDevice & device) {
		
		string ret = "<device>";
		ret.append(NEWLINE);

		// device properties

		PropertyMap & props = device.getProperties();
		for (size_t i = 0; i < props.size(); i++) {
			ret.append(XmlUtils::toNameValueTag(props[i]));
			ret.append(NEWLINE);
		}

		ret.append("<serviceList>");
		ret.append(NEWLINE);

		// services
		
		vector<AutoRef<UPnPService> > & services = device.services();
		for (vector<AutoRef<UPnPService> >::iterator iter = services.begin(); iter != services.end(); iter++) {
			AutoRef<UPnPService> service = *iter;
			
			ret.append("<service>");
			ret.append(NEWLINE);
			PropertyMap & props = service->getProperties();
			for (size_t i = 0; i < props.size(); i++) {
				ret.append(XmlUtils::toNameValueTag(props[i]));
				ret.append(NEWLINE);
			}
			ret.append("</service>");
			ret.append(NEWLINE);
		}
		
		ret.append("</serviceList>");
		ret.append(NEWLINE);

		// embedded devices

		if (device.embeddedDevices().size() > 0) {
			ret.append("<deviceList>");
			ret.append(NEWLINE);

			for (size_t i = 0; i < device.embeddedDevices().size(); i++) {
				ret.append(serializeDevice(*device.embeddedDevices()[i]));
			}
			
			ret.append("</deviceList>");
			ret.append(NEWLINE);
		}
		
		ret.append("</device>");
		ret.append(NEWLINE);

		return ret;
	}
	
	string UPnPDeviceSerializer::serializeScpd(UPnPScpd & scpd) {
		string ret = FIRSTLINE;
		ret.append(NEWLINE);
		ret.append("<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">");
		ret.append(NEWLINE);
		ret.append("<specVersion>");
		ret.append(NEWLINE);
		ret.append("<major>1</major>");
		ret.append(NEWLINE);
		ret.append("<minor>0</minor>");
		ret.append(NEWLINE);
		ret.append("</specVersion>");
		ret.append(NEWLINE);
		
		ret.append("<actionList>");
		ret.append(NEWLINE);
		for (size_t i = 0; i < scpd.actions().size(); i++) {
			ret.append(serializeAction(scpd.actions()[i]));
		}
		ret.append("</actionList>");
		ret.append(NEWLINE);

		ret.append("<serviceStateTable>");
		ret.append(NEWLINE);
		for (size_t i = 0; i < scpd.stateVariables().size(); i++) {
			ret.append(serializeStateVariable(scpd.stateVariables()[i]));
		}
		ret.append("</serviceStateTable>");
		ret.append(NEWLINE);

		ret.append("</scpd>");
		ret.append(NEWLINE);
		return ret;
	}

	string UPnPDeviceSerializer::serializeAction(UPnPAction & action) {
		string ret = "<action>";
		ret.append(NEWLINE);

		ret.append(XmlUtils::toNameValueTag("name", action.name()));
		ret.append(NEWLINE);
		ret.append("<argumentList>");
		ret.append(NEWLINE);
		for (size_t i = 0; i < action.arguments().size(); i++) {
			ret.append(serializeArgument(action.arguments()[i]));
		}
		ret.append("</argumentList>");
		ret.append(NEWLINE);
		ret.append("</action>");
		ret.append(NEWLINE);

		return ret;
	}
	string UPnPDeviceSerializer::serializeArgument(UPnPArgument & argument) {
		string ret = "<argument>";
		ret.append(NEWLINE);
		ret.append(XmlUtils::toNameValueTag("name", argument.name()));
		ret.append(NEWLINE);
		ret.append(XmlUtils::toNameValueTag("direction", argument.in() ? "in" : "out"));
		ret.append(NEWLINE);
		ret.append(XmlUtils::toNameValueTag("relatedStateVariable", argument.relatedStateVariable()));
		ret.append(NEWLINE);
		ret.append("</argument>");
		ret.append(NEWLINE);
		return ret;
	}
	string UPnPDeviceSerializer::serializeStateVariable(UPnPStateVariable & stateVariable) {
		
		string ret = "<stateVariable ";
		
		// send events
		ret.append("sendEvents=");
		ret.append(Text::quote(stateVariable.sendEvents() ? "yes" : "no", "\""));

		// multicast
		if (stateVariable.multicast()) {
			ret.append(" ");
			ret.append("multicast=");
			ret.append(Text::quote(stateVariable.multicast() ? "yes" : "no", "\""));
		}
		ret.append(">");
		ret.append(NEWLINE);

		// name
		ret.append(XmlUtils::toNameValueTag("name", stateVariable.name()));
		ret.append(NEWLINE);

		// data type
		ret.append(XmlUtils::toNameValueTag("dataType", stateVariable.dataType()));
		ret.append(NEWLINE);

		// allowed value list
		if (stateVariable.allowedValueList().size() > 0) {
			ret.append("<allowedValueList>");
			ret.append(NEWLINE);
			for (size_t i = 0; i < stateVariable.allowedValueList().size(); i++) {
				ret.append(XmlUtils::toNameValueTag("allowedValue", stateVariable.allowedValueList()[i]));
				ret.append(NEWLINE);
			}
			ret.append("</allowedValueList>");
			ret.append(NEWLINE);
		}

		// allowed value range
		if (!stateVariable.minimum().empty()) {
			ret.append("<allowedValueRange>");
			ret.append(NEWLINE);
			ret.append(XmlUtils::toNameValueTag("minimum", stateVariable.minimum()));
			ret.append(NEWLINE);
			ret.append(XmlUtils::toNameValueTag("maximum", stateVariable.maximum()));
			ret.append(NEWLINE);
			if (!stateVariable.step().empty()) {
				ret.append(XmlUtils::toNameValueTag("step", stateVariable.step()));
				ret.append(NEWLINE);
			}
			ret.append("</allowedValueRange>");
			ret.append(NEWLINE);
		}

		ret.append("</stateVariable>");
		ret.append(NEWLINE);

		return ret;
	}
}
