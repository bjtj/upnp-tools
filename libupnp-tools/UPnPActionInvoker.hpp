#ifndef __UPNP_ACTION_INVOKER_HPP__
#define __UPNP_ACTION_INVOKER_HPP__

#include <string>
#include <libhttp-server/Url.hpp>
#include "UPnPActionRequest.hpp"
#include "UPnPActionResponse.hpp"

namespace upnp {
	
	class UPnPActionInvoker {
	private:
		http::Url _controlUrl;
		
	public:
		UPnPActionInvoker(http::Url controlUrl);
		virtual ~UPnPActionInvoker();
		http::Url & controlUrl();
		UPnPActionResponse invoke(UPnPActionRequest & request);
	};
}

#endif
