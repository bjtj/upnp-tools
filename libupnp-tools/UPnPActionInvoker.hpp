#ifndef __UPNP_ACTION_INVOKER_HPP__
#define __UPNP_ACTION_INVOKER_HPP__

#include <string>
#include <map>
#include <vector>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/StringElements.hpp>
#include <liboslayer/XmlParser.hpp>
#include <libhttp-server/Url.hpp>
#include "HttpUtils.hpp"
#include "XmlUtils.hpp"

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
		std::string makeSoapRequestContent(UPnPActionRequest & request);
	};
}

#endif
