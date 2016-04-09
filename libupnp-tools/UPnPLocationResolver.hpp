#ifndef __UPNP_LOCATION_RESOLVER_HPP__
#define __UPNP_LOCATION_RESOLVER_HPP__

#include <string>

namespace UPNP {

	/**
	 * @brief 
	 */
	class UPnPLocationResolver {
	private:
	public:
		UPnPLocationResolver() {}
		virtual ~UPnPLocationResolver() {}
		virtual std::string resolve(const std::string & loc) = 0;
	};
}

#endif
