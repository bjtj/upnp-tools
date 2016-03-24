#ifndef __UPNP_EVENT_SUBSCRIBER_HPP__
#define __UPNP_EVENT_SUBSCRIBER_HPP__

#include <vector>
#include <map>
#include <string>
#include <libhttp-server/Url.hpp>

namespace UPNP {

	/**
	 * @brief
	 */
	class UPnPEventNotify {
	private:
		std::string _sid;
		unsigned long _seq;
		std::map<std::string, std::string> props;
	public:
		UPnPEventNotify();
		virtual ~UPnPEventNotify();
		std::string & sid();
		unsigned long & seq();
		std::vector<std::string> propertyNames();
		std::string & operator[] (const std::string & name);
	};


	/**
	 * @brief
	 */
	class UPnPEventNotifyListener {
	private:
	public:
		UPnPEventNotifyListener();
		virtual ~UPnPEventNotifyListener();
		virtual void onNotify(UPnPEventNotify & notify) = 0;
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
		UPnPEventSubscriber(HTTP::Url & eventSubUrl);
		virtual ~UPnPEventSubscriber();
		UPnPEventSubscribeResponse subscribe(UPnPEventSubscribeRequest & request);
		void unsubscribe(const std::string & sid);
	};
	
}

#endif
