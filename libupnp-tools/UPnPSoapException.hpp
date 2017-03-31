#ifndef __UPNP_SOAP_EXCEPTION_HPP__
#define __UPNP_SOAP_EXCEPTION_HPP__

#include "UPnPExceptions.hpp"
#include "UPnPSoapFormatter.hpp"
#include <string>

namespace UPNP {

	/**
	 * UPnP Soap Exception
	 */
	class UPnPSoapException : public UPnPException {
	public:
		explicit UPnPSoapException(int code)
			: UPnPException(UPnPActionErrorCodes::getDescription(code), code, 0)
			{/**/}
		explicit UPnPSoapException(int code, std::string & description)
			: UPnPException(description, code, 0)
			{/**/}
		virtual ~UPnPSoapException() throw() {/**/}
		std::string getSoapErrorMessage() {
			return UPnPSoapFormatter::formatError(error_code(), toString());
		}
	};
}

#endif
