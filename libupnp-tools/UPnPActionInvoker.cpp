#include "UPnPActionInvoker.hpp"
#include "UPnPDebug.hpp"

namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace HTTP;
	using namespace UTIL;
	using namespace XML;
	

	UPnPActionInvoker::UPnPActionInvoker(Url controlUrl) : _controlUrl(controlUrl) {
	}
		
	UPnPActionInvoker::~UPnPActionInvoker() {}

	Url & UPnPActionInvoker::controlUrl() {
		return _controlUrl;
	}
		
	UPnPActionResponse UPnPActionInvoker::invoke(UPnPActionRequest & request) {

		UPnPActionResponse response;
		string serviceType = request.serviceType();
		string actionName = request.actionName();
			
		LinkedStringMap headers;
		headers["SOAPACTION"] = ("\"" + serviceType + "#" + actionName + "\"");
		string soapRequest = makeSoapRequestContent(request);
		UPnPDebug::instance().debug("upnp:action/request", soapRequest);
		string result = HttpUtils::httpPost(_controlUrl, headers, soapRequest);
		UPnPDebug::instance().debug("upnp:action/response", result);
		XmlDocument soapResponse = DomParser::parse(result);
		if (soapResponse.getRootNode().nil()) {
			throw Exception("invoke error / wrong response format", -1, 0);
		}
		string actionNameResponse = actionName + "Response";
		AutoRef<XmlNode> node = soapResponse.getRootNode()->getElementByTagName(actionNameResponse);
		if (node.nil()) {
			throw Exception("invoke error / wrong response format - no action response", -1, 0);
		}
		vector<AutoRef<XmlNode> > children = node->children();
		for (vector<AutoRef<XmlNode> >::iterator iter = children.begin();
			 iter != children.end(); iter++)
		{
			if (XmlUtils::testKeyValueXmlNode(*iter)) {
				KeyValue nv = XmlUtils::toKeyValue(*iter);
				response[XmlDecoder::decode(nv.key())] = XmlDecoder::decode(nv.value());
			}
		}
		return response;
	}
		
	string UPnPActionInvoker::makeSoapRequestContent(UPnPActionRequest & request) {

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

}
