#ifndef __UPNP_ACTION_INVOKER_HPP__
#define __UPNP_ACTION_INVOKER_HPP__

#include <string>
#include <map>
#include <vector>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/StringElement.hpp>
#include <liboslayer/XmlParser.hpp>
#include <libhttp-server/Url.hpp>
#include "HttpUtils.hpp"
#include "XmlUtils.hpp"

namespace UPNP {
	
	class UPnPActionInvoker {
	private:
		HTTP::Url _baseUrl;
		UTIL::AutoRef<UPnPService> service;
		std::string _actionName;
		std::map<std::string, std::string> _inParams;
		std::map<std::string, std::string> _outParams;
		
	public:
		UPnPActionInvoker(HTTP::Url baseUrl, UTIL::AutoRef<UPnPService> service) : _baseUrl(baseUrl), service(service) {}
		virtual ~UPnPActionInvoker() {}

		HTTP::Url & baseUrl() {return _baseUrl;}
		UTIL::AutoRef<UPnPService> getService() {return service;}
		void setService(UTIL::AutoRef<UPnPService> service) {this->service = service;}
		std::string & actionName() {return _actionName;}
		std::map<std::string, std::string> & inParams() {return _inParams;}
		std::map<std::string, std::string> & outParams() {return _outParams;}
		void invoke() {
			UTIL::LinkedStringMap headers;
			headers["SOAPACTION"] = ("\"" + service->getServiceType() + "#" + _actionName + "\"");
			HTTP::Url url = _baseUrl.relativePath(service->getControlUrl());
			std::string result = HttpUtils::httpPost(url, headers, makeSoapRequestContent());
			XML::XmlDocument doc = XML::DomParser::parse(result);
			if (doc.getRootNode().nil()) {
				return;
			}
			XML::XmlNode * node = doc.getRootNode()->getElementByTagName(_actionName + "Response");
			if (node) {
				std::vector<XML::XmlNode*> children = node->children();
				for (std::vector<XML::XmlNode*>::iterator iter = children.begin(); iter != children.end(); iter++) {
					if (XmlUtils::testNameValueXmlNode(*iter)) {
						UTIL::NameValue nv = XmlUtils::toNameValue(*iter);
						_outParams[nv.name()] = nv.value();
					}
				}
			}
		}
		std::string makeSoapRequestContent() {
			std::string request;
			request = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n";
			request.append("<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n");
			request.append("<s:Body>\r\n");
			request.append("<u:" + _actionName + " xmlns:u=\"" + service->getServiceType() + "\">\r\n");
			for (std::map<std::string, std::string>::iterator iter = _inParams.begin(); iter != _inParams.end(); iter++) {
				std::string name = iter->first;
				std::string & value = iter->second;
				request.append("<" + name + ">" + XML::XmlEncoder::encode(value) + "</" + name + ">\r\n");
			}
			request.append("</u:" + _actionName + ">\r\n");
			request.append("</s:Body>\r\n");
			request.append("</s:Envelope>");
			return request;
		}
	};

	
}

#endif
