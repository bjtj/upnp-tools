#ifndef __UPNP_EVENT_SUBSCRIBER_HPP__
#define __UPNP_EVENT_SUBSCRIBER_HPP__

#include <vector>
#include <map>
#include <string>
#include <libhttp-server/Url.hpp>
#include "UPnPEventSubscription.hpp"

namespace UPNP {

	/**
	 * @brief
	 */
	class UPnPNotify {
	private:
		std::string _sid;
		unsigned long _seq;
		std::map<std::string, std::string> props;
		UPnPEventSubscription _subscription;
		
	public:
		UPnPNotify();
		UPnPNotify(const std::string & sid);
		UPnPNotify(const std::string & sid, unsigned long seq);
		virtual ~UPnPNotify();
		std::string & sid();
		unsigned long & seq();
		std::vector<std::string> propertyNames();
		std::string & operator[] (const std::string & name);
		UPnPEventSubscription & subscription();
	};


	/**
	 * @brief
	 */
	class UPnPNotificationListener {
	private:
	public:
		UPnPNotificationListener();
		virtual ~UPnPNotificationListener();
		virtual void onNotify(UPnPNotify & notify) = 0;
	};


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
		HTTP::Url eventSubUrl;
	public:
		UPnPEventSubscriber(const HTTP::Url & eventSubUrl);
		virtual ~UPnPEventSubscriber();
		UPnPEventSubscribeResponse subscribe(UPnPEventSubscribeRequest & request);
		void unsubscribe(const std::string & sid);
	};
	
}

#endif