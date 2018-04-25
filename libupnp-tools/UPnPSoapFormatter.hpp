#ifndef __UPNP_SOAP_FORMATTER_HPP__
#define __UPNP_SOAP_FORMATTER_HPP__

#include "UPnPActionRequest.hpp"
#include "UPnPActionResponse.hpp"
#include <string>

namespace upnp {

	/**
	 * 
	 */
	class UPnPSoapFormatter {
	public:
		UPnPSoapFormatter();
		virtual ~UPnPSoapFormatter();
		static std::string formatRequest(UPnPActionRequest & request);
		static std::string formatResponse(UPnPActionResponse & response);
		static std::string formatError(int errorCode);
		static std::string formatError(int errorCode, const std::string & errorString);
		static UPnPActionResponse parseResponse(const std::string & actionName, const std::string & xml);
	};
}

#endif
