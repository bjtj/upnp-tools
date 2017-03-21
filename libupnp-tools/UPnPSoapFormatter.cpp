#include "UPnPSoapFormatter.hpp"
#include "UPnPActionErrorCodes.hpp"
#include <liboslayer/Text.hpp>
#include <liboslayer/StringElements.hpp>
#include <liboslayer/XmlEncoderDecoder.hpp>

namespace UPNP {

	using namespace std;
	using namespace UTIL;

	UPnPSoapFormatter::UPnPSoapFormatter() {
	}
	UPnPSoapFormatter::~UPnPSoapFormatter() {
	}
	string UPnPSoapFormatter::formatResponse(UPnPActionResponse & response) {
		string actionName = response.actionName();
		string serviceType = response.serviceType();
		LinkedStringMap & parameters = response.parameters();
			
		string xml;
		xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n";
		xml.append("<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" "
				   "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n");
		xml.append("<s:Body>\r\n");
		xml.append("<u:" + actionName + "Response xmlns:u=\"" + serviceType + "\">\r\n");
		for (size_t i = 0; i < parameters.size(); i++) {
			KeyValue & kv = parameters[i];
			string name = XML::XmlEncoder::encode(kv.key());
			string value = XML::XmlEncoder::encode(kv.value());
			xml.append("<" + name + ">" + value + "</" + name + ">\r\n");
		}
		xml.append("</u:" + actionName + "Response>");
		xml.append("</s:Body>\r\n");
		xml.append("</s:Envelope>");
		return xml;
	}
	
	string UPnPSoapFormatter::formatError(int errorCode) {
		return formatError(errorCode, UPnPActionErrorCodes::getDescription(errorCode));
	}
	
	string UPnPSoapFormatter::formatError(int errorCode, const string & errorString) {
		string xml;
		xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n";
		xml.append("<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" "
				   "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n");
		xml.append("<s:Body>\r\n");
		xml.append("<s:Fault>");
		xml.append("<faultcode>s:Client</faultcode>");
		xml.append("<faultstring>UPnPError</faultstring>");
		xml.append("<detail>");
		xml.append("<UPnPError xmlns=\"urn:schemas-upnp-org:control-1-0\">");
		xml.append("<errorCode>" + Text::toString(errorCode) + "</errorCode>");
		xml.append("<errorDescription>" + errorString + "</errorDescription>");
		xml.append("</UPnPError>");
		xml.append("</detail>");
		xml.append("</s:Fault>");
		xml.append("</s:Body>\r\n");
		xml.append("</s:Envelope>");
		return xml;
	}
	
}
