#ifndef __UPNP_EXCEPTION_HPP__
#define __UPNP_EXCEPTION_HPP__

#include <liboslayer/os.hpp>

namespace upnp {

	DECL_NAMED_EXCEPTION(UPnPException);
	DECL_EXCEPTION(UPnPParseException, UPnPException);
}

#endif
