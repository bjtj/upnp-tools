#ifndef __UPNP_ACTION_INVOKER_HPP__
#define __UPNP_ACTION_INVOKER_HPP__

#include <string>
#include <libhttp-server/Url.hpp>
#include "UPnPActionRequest.hpp"
#include "UPnPActionResponse.hpp"

namespace UPNP {
	
	class UPnPActionInvoker {
	private:
		HTTP::Url _controlUrl;
		
	public:
		UPnPActionInvoker(HTTP::Url controlUrl);
		virtual ~UPnPActionInvoker();
		HTTP::Url & controlUrl();
		UPnPActionResponse invoke(UPnPActionRequest & request);
	};
}

#endif
