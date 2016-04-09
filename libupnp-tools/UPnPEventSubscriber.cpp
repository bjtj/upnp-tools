#include "UPnPEventSubscriber.hpp"
#include <liboslayer/StringElement.hpp>
#include <liboslayer/Text.hpp>
#include <libhttp-server/HttpHeader.hpp>
#include "HttpUtils.hpp"

namespace UPNP {

	using namespace std;
	using namespace HTTP;
	using namespace UTIL;

	/**
	 *
	 */

	UPnPNotify::UPnPNotify() : _seq(0) {
	}
	UPnPNotify::UPnPNotify(const string & sid) : _sid(sid), _seq(0) {
	}
	UPnPNotify::UPnPNotify(const string & sid, unsigned long seq) : _sid(sid), _seq(seq) {
	}
	UPnPNotify::~UPnPNotify() {
	}
	std::string & UPnPNotify::sid() {
		return _sid;
	}
	unsigned long & UPnPNotify::seq() {
		return _seq;
	}
	std::vector<std::string> UPnPNotify::propertyNames() {
		std::vector<std::string> names;
		for (std::map<std::string, std::string>::iterator iter = props.begin(); iter != props.end(); iter++) {
			names.push_back(iter->first);
		}
		return names;
	}
	std::string & UPnPNotify::operator[] (const std::string & name) {
		return props[name];
	}
	UPnPEventSubscription & UPnPNotify::subscription() {
		return _subscription;
	}

	/**
	 *
	 */

	UPnPNotificationListener::UPnPNotificationListener() {
	}
	UPnPNotificationListener::~UPnPNotificationListener() {
	}

	/**
	 *
	 */

	UPnPEventSubscribeRequest::UPnPEventSubscribeRequest() : _timeoutSec(0) {
	}
	UPnPEventSubscribeRequest::UPnPEventSubscribeRequest(std::vector<std::string> & callbackUrls, unsigned long timeoutSec) : _callbackUrls(callbackUrls), _timeoutSec(timeoutSec) {
	}
	UPnPEventSubscribeRequest::UPnPEventSubscribeRequest(const std::string & callbackUrl, unsigned long timeoutSec) : _timeoutSec(timeoutSec) {
		_callbackUrls.push_back(callbackUrl);
	}
	UPnPEventSubscribeRequest::~UPnPEventSubscribeRequest() {
	}
	std::vector<std::string> & UPnPEventSubscribeRequest::callbackUrls() {
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
	std::string & UPnPEventSubscribeResponse::sid() {
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
		string callbackUrlStrings;
		vector<string> & callbackUrls = request.callbackUrls();
		for (vector<string>::iterator iter = callbackUrls.begin(); iter != callbackUrls.end(); iter++) {
			if (callbackUrlStrings.size() > 0) {
				callbackUrlStrings.append(" ");
			}
			callbackUrlStrings.append("<" + *iter + ">");
		}
		headers["CALLBACK"] = callbackUrlStrings;
		headers["NT"] = "upnp:event";
		headers["TIMEOUT"] = string("Second-") + Text::toString(request.timeoutSec()) ;
		HttpResponseHeader header = HttpUtils::dumpHttpRequest(eventSubUrl, "SUBSCRIBE", headers).getResponseHeader();
		response.sid() = header["SID"];
		return response;
	}
	void UPnPEventSubscriber::unsubscribe(const std::string & sid) {
		UTIL::LinkedStringMap headers;
		headers["SID"] = sid;
		HttpUtils::dumpHttpRequest(eventSubUrl, "UNSUBSCRIBE", headers).getResponseHeader().getStatusCode();
	}
}