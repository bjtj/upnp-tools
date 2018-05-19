#include "UPnPEventSubscriber.hpp"
#include "HttpUtils.hpp"
#include "UPnPModels.hpp"
#include <liboslayer/StringElements.hpp>
#include <liboslayer/Text.hpp>
#include <libhttp-server/HttpHeader.hpp>


namespace upnp {

	using namespace std;
	using namespace osl;
	using namespace http;

	/**
	 *
	 */

	UPnPEventSubscribeRequest::UPnPEventSubscribeRequest() : _timeoutSec(0) {
	}
	UPnPEventSubscribeRequest::UPnPEventSubscribeRequest(vector<string> & callbackUrls,
														 unsigned long timeoutSec)
		: _callbackUrls(callbackUrls), _timeoutSec(timeoutSec)
	{
	}
	
	UPnPEventSubscribeRequest::UPnPEventSubscribeRequest(const string & callbackUrl,
														 unsigned long timeoutSec)
		: _timeoutSec(timeoutSec)
	{
		_callbackUrls.push_back(callbackUrl);
	}
	
	UPnPEventSubscribeRequest::~UPnPEventSubscribeRequest() {
	}
	
	vector<string> & UPnPEventSubscribeRequest::callbackUrls() {
		return _callbackUrls;
	}
	
	unsigned long UPnPEventSubscribeRequest::timeoutSec() {
		return _timeoutSec;
	}

	/**
	 *
	 */

	UPnPEventSubscribeResponse::UPnPEventSubscribeResponse() {
	}
	
	UPnPEventSubscribeResponse::~UPnPEventSubscribeResponse() {
	}
	
	string & UPnPEventSubscribeResponse::sid() {
		return _sid;
	}

	/**
	 *
	 */

	UPnPEventSubscriber::UPnPEventSubscriber(const Url & eventSubUrl) : eventSubUrl(eventSubUrl) {
	}
	
	UPnPEventSubscriber::~UPnPEventSubscriber() {
	}

	UPnPEventSubscribeResponse UPnPEventSubscriber::subscribe(UPnPEventSubscribeRequest & request) {
		UPnPEventSubscribeResponse response;
		LinkedStringMap headers;
		CallbackUrls cbs(request.callbackUrls());
		headers["CALLBACK"] = cbs.toString();
		headers["NT"] = "upnp:event";
		headers["TIMEOUT"] = string("Second-") + Text::toString(request.timeoutSec()) ;
		HttpResponseHeader header = HttpUtils::httpRequest(eventSubUrl, "SUBSCRIBE", headers)
			.getResponseHeader();
		response.sid() = header["SID"];
		// TODO: use TIMEOUT value in response
		return response;
	}
	void UPnPEventSubscriber::unsubscribe(const string & sid) {
		LinkedStringMap headers;
		headers["SID"] = sid;
		HttpUtils::httpRequest(eventSubUrl, "UNSUBSCRIBE", headers).getResponseHeader().getStatusCode();
	}
}
