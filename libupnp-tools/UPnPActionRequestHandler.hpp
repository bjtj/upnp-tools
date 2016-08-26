#ifndef __UPNP_ACTION_HANDLER_HPP__
#define __UPNP_ACTION_HANDLER_HPP__

#include "UPnPActionRequest.hpp"
#include "UPnPActionResponse.hpp"

namespace UPNP {
	
	class UPnPActionRequestHandler {
	private:
	public:
		UPnPActionRequestHandler() {}
		virtual ~UPnPActionRequestHandler() {}
		virtual void handleActionRequest(UPnPActionRequest & request, UPnPActionResponse & response) = 0;
	};
}

#endif
