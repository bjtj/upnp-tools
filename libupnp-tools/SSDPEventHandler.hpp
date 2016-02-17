#ifndef __SSDP_EVENT_HANDLER_HPP__
#define __SSDP_EVENT_HANDLER_HPP__

#include <libhttp-server/HttpHeader.hpp>
#include "SSDPHeader.hpp"

namespace SSDP {
	/**
	 * @brief notify handler
	 */
	class OnNotifyHandler {
	private:
	public:
		OnNotifyHandler() {}
		virtual ~OnNotifyHandler() {}

		virtual void onNotify(const HTTP::HttpHeader & header) = 0;
	};

	/**
	 * @brief msearch handler
	 */
	class OnMsearchHandler {
	private:
	public:
		OnMsearchHandler() {}
		virtual ~OnMsearchHandler() {}

        virtual void onMsearch(const HTTP::HttpHeader & header, const OS::InetAddress & remoteAddr) = 0;
	};

	/**
	 *
	 */
	class OnHttpResponseHandler {
	private:
	public:
		OnHttpResponseHandler() {}
		virtual ~OnHttpResponseHandler() {}

		virtual void onHttpResponse(const HTTP::HttpHeader & header) = 0;
	};

	/**
	 *
	 */
	class SSDPEventHandler {
	private:
	public:
		SSDPEventHandler() {}
		virtual ~SSDPEventHandler() {}

        virtual bool filter(SSDPHeader & header) {return true;}
		virtual void onMsearch(SSDPHeader & header) {}
		virtual void onNotify(SSDPHeader & header) {}
		virtual void onMsearchResponse(SSDPHeader & header) {}
	};
}

#endif
