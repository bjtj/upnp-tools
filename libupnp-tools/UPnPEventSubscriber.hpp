#ifndef __UPNP_EVENT_SUBSCRIBER_HPP__
#define __UPNP_EVENT_SUBSCRIBER_HPP__

#include <vector>
#include <map>
#include <string>
#include <libhttp-server/Url.hpp>
#include "UPnPEventSubscription.hpp"

namespace upnp {

	/**
	 * @brief
	 */
	class UPnPEventSubscribeRequest {
	private:
		std::vector<std::string> _callbackUrls;
		unsigned long _timeoutSec;
	public:
		UPnPEventSubscribeRequest();
		UPnPEventSubscribeRequest(std::vector<std::string> & callbackUrls, unsigned long timeoutSec);
		UPnPEventSubscribeRequest(const std::string & callbackUrl, unsigned long timeoutSec);
		virtual ~UPnPEventSubscribeRequest();
		std::vector<std::string> & callbackUrls();
		unsigned long timeoutSec();
	};

	/**
	 * @brief
	 */
	class UPnPEventSubscribeResponse {
	private:
		std::string _sid;
	public:
		UPnPEventSubscribeResponse();
		virtual ~UPnPEventSubscribeResponse();
		std::string & sid();
	};

	/**
	 * @brief
	 */
	class UPnPEventSubscriber {
	private:
		http::Url eventSubUrl;
	public:
		UPnPEventSubscriber(const http::Url & eventSubUrl);
		virtual ~UPnPEventSubscriber();
		UPnPEventSubscribeResponse subscribe(UPnPEventSubscribeRequest & request);
		void unsubscribe(const std::string & sid);
	};
	
}

#endif
