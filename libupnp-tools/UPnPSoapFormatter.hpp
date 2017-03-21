#ifndef __UPNP_SOAP_FORMATTER_HPP__
#define __UPNP_SOAP_FORMATTER_HPP__

#include "UPnPActionResponse.hpp"
#include <string>

namespace UPNP {

	/**
	 * 
	 */
	class UPnPSoapFormatter {
	public:
		UPnPSoapFormatter();
		virtual ~UPnPSoapFormatter();
		static std::string formatResponse(UPnPActionResponse & response);
		static std::string formatError(int errorCode);
		static std::string formatError(int errorCode, const std::string & errorString);
	};
}

#endif
