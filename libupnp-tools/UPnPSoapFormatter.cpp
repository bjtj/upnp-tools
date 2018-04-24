#include "UPnPSoapFormatter.hpp"
#include "UPnPActionErrorCodes.hpp"
#include "XmlUtils.hpp"
#include <liboslayer/Text.hpp>
#include <liboslayer/StringElements.hpp>
#include <liboslayer/XmlEncoderDecoder.hpp>

namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace UTIL;
	using namespace XML;


	UPnPSoapFormatter::UPnPSoapFormatter() {
	}

	UPnPSoapFormatter::~UPnPSoapFormatter() {
	}

	string UPnPSoapFormatter::formatRequest(UPnPActionRequest & request) {

		string actionName = request.actionName();
		string serviceType = request.serviceType();
		map<string, string> & parameters = request.parameters();
			
		string xml;
		xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n";
		xml.append("<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" "
				   "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n");
		xml.append("<s:Body>\r\n");
		xml.append("<u:" + actionName + " xmlns:u=\"" + serviceType + "\">\r\n");
		for (map<string, string>::iterator iter = parameters.begin();
			 iter != parameters.end(); iter++)
		{
			string name = XmlEncoder::encode(iter->first);
			string value = XmlEncoder::encode(iter->second);
			xml.append("<" + name + ">" + value + "</" + name + ">\r\n");
		}
		xml.append("</u:" + actionName + ">\r\n");
		xml.append("</s:Body>\r\n");
		xml.append("</s:Envelope>");
		return xml;
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

	UPnPActionResponse UPnPSoapFormatter::parseResponse(const string & actionName, const string & xml) {
		UPnPActionResponse response;
		XmlDocument soapResponse = DomParser::parse(xml);
		if (soapResponse.rootNode().nil()) {
			throw Exception("invoke error / wrong response format");
		}
		string actionNameResponse = actionName + "Response";
		AutoRef<XmlNode> node = soapResponse.rootNode()->getElementByTagName(actionNameResponse);
		if (node.nil()) {
			throw Exception("invoke error / wrong response format - no action response");
		}
		vector< AutoRef<XmlNode> > children = node->children();
		for (vector< AutoRef<XmlNode> >::iterator iter = children.begin();
			 iter != children.end(); iter++)
		{
			if (XmlUtils::testKeyValueXmlNode(*iter)) {
				KeyValue nv = XmlUtils::toKeyValue(*iter);
				response[XmlDecoder::decode(nv.key())] = XmlDecoder::decode(nv.value());
			}
		}
		return response;
		
	}
}
