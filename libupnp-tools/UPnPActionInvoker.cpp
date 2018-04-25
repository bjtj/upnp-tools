#include "UPnPActionInvoker.hpp"
#include "UPnPDebug.hpp"
#include "UPnPSoapFormatter.hpp"
#include "HttpUtils.hpp"


namespace upnp {

	using namespace std;
	using namespace osl;
	using namespace http;

	
	UPnPActionInvoker::UPnPActionInvoker(Url controlUrl) : _controlUrl(controlUrl) {
	}
		
	UPnPActionInvoker::~UPnPActionInvoker() {}

	Url & UPnPActionInvoker::controlUrl() {
		return _controlUrl;
	}
		
	UPnPActionResponse UPnPActionInvoker::invoke(UPnPActionRequest & request) {

		string serviceType = request.serviceType();
		string actionName = request.actionName();
			
		LinkedStringMap headers;
		headers["SOAPACTION"] = ("\"" + serviceType + "#" + actionName + "\"");
		string soapRequest = UPnPSoapFormatter::formatRequest(request);
		UPnPDebug::instance().debug("upnp:action/request", soapRequest);
		string soapResponse = HttpUtils::httpPost(_controlUrl, headers, soapRequest);
		UPnPDebug::instance().debug("upnp:action/response", soapResponse);
		return UPnPSoapFormatter::parseResponse(actionName, soapResponse);
	}
}
