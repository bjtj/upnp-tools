#ifndef __UPNP_ACTION_HANDLER_HPP__
#define __UPNP_ACTION_HANDLER_HPP__

#include "UPnPActionRequest.hpp"
#include "UPnPActionResponse.hpp"

namespace upnp {
	
	class UPnPActionRequestHandler {
	private:
	public:
		UPnPActionRequestHandler() {}
		virtual ~UPnPActionRequestHandler() {}
		virtual bool handleActionRequest(UPnPActionRequest & request, UPnPActionResponse & response) = 0;
	};
}

#endif
