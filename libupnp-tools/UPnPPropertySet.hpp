#ifndef __UPNP_PROPERTY_SET_HPP__
#define __UPNP_PROPERTY_SET_HPP__

#include <vector>
#include <map>
#include <string>
#include "UPnPEventSubscription.hpp"


namespace upnp {

	/**
	 * @brief
	 */
	class UPnPPropertySet {
	private:
		std::string _sid;
		unsigned long _seq;
		std::map<std::string, std::string> _props;
		UPnPEventSubscription _subscription;
		
	public:
		UPnPPropertySet();
		UPnPPropertySet(const std::string & sid);
		UPnPPropertySet(const std::string & sid, unsigned long seq);
		virtual ~UPnPPropertySet();
		std::string & sid();
		const std::string & sid() const;
		unsigned long & seq();
		const unsigned long & seq() const;
		std::vector<std::string> propertyNames();
		std::string & operator[] (const std::string & name);
		UPnPEventSubscription & subscription();
	};

}

#endif
