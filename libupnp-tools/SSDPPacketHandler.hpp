#ifndef __SSPD_PACKET_HANDLER_HPP__
#define __SSPD_PACKET_HANDLER_HPP__

#include "SSDPHeader.hpp"

namespace SSDP {

	class SSDPPacketHandler {
	private:
	public:
		SSDPPacketHandler() {}
		virtual ~SSDPPacketHandler() {}

		virtual void onAnyPacket(SSDPHeader & header) {}
		virtual void onMsearch(SSDPHeader & header) {}
		virtual void onNotify(SSDPHeader & header) {}
		virtual void onMsearchResponse(SSDPHeader & header) {}
	};
}

#endif