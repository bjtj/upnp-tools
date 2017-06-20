#ifndef __SSDP_EVENT_LISTENER_HPP__
#define __SSDP_EVENT_LISTENER_HPP__

#include <libhttp-server/HttpHeader.hpp>
#include "SSDPHeader.hpp"

namespace SSDP {

	/**
	 *
	 */
	class SSDPEventListener {
	private:
	public:
		SSDPEventListener() {}
		virtual ~SSDPEventListener() {}

        virtual bool filter(const SSDPHeader & header) {return true;}
		virtual void onMsearch(const SSDPHeader & header) {}
		virtual void onNotify(const SSDPHeader & header) {}
		virtual void onMsearchResponse(const SSDPHeader & header) {}

		void dispatch(SSDPHeader & header) {
			if (filter(header)) {
				if (header.isMsearch()) {
					onMsearch(header);
				} else if (header.isNotify()) {
					onNotify(header);
				} else if (header.isSSDPResponse()) {
					onMsearchResponse(header);
				}
			}
		}
	};
}

#endif
