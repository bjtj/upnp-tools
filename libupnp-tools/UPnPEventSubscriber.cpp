#include "UPnPEventSubscriber.hpp"

namespace UPNP {

	using namespace std;
	using namespace HTTP;

	UPnPEventNotify::UPnPEventNotify() {
	}
	UPnPEventNotify::~UPnPEventNotify() {
	}
	std::string & UPnPEventNotify::sid() {
		return _sid;
	}
	unsigned long & UPnPEventNotify::seq() {
		return _seq;
	}
	std::vector<std::string> UPnPEventNotify::propertyNames() {
		std::vector<std::string> names;
		for (std::map<std::string, std::string>::iterator iter = props.begin(); iter != props.end(); iter++) {
			names.push_back(iter->first);
		}
		return names;
	}
	std::string & UPnPEventNotify::operator[] (const std::string & name) {
		return props[name];
	}

	

	UPnPEventNotifyListener::UPnPEventNotifyListener() {
	}
	UPnPEventNotifyListener::~UPnPEventNotifyListener() {
	}


	UPnPEventSubscribeRequest::UPnPEventSubscribeRequest() {
	}
	UPnPEventSubscribeRequest::~UPnPEventSubscribeRequest() {
	}
	std::vector<std::string> & UPnPEventSubscribeRequest::callbackUrls() {
		return _callbackUrls;
	}
	unsigned long UPnPEventSubscribeRequest::timeoutSec() {
		return _timeoutSec;
	}


	UPnPEventSubscribeResponse::UPnPEventSubscribeResponse() {
	}
	UPnPEventSubscribeResponse::~UPnPEventSubscribeResponse() {
	}
	std::string & UPnPEventSubscribeResponse::sid() {
		return _sid;
	}



	UPnPEventSubscriber::UPnPEventSubscriber(Url & eventSubUrl) : eventSubUrl(eventSubUrl) {
	}
	UPnPEventSubscriber::~UPnPEventSubscriber() {
	}

	UPnPEventSubscribeResponse UPnPEventSubscriber::subscribe(UPnPEventSubscribeRequest & request) {
		UPnPEventSubscribeResponse response;
		return response;
	}
	void UPnPEventSubscriber::unsubscribe(const std::string & sid) {
	}
	
}
