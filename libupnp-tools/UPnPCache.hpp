#ifndef __UPNP_CACHE_HPP__
#define __UPNP_CACHE_HPP__

namespace UPNP {
	
	class UPnPCache {
	private:
		unsigned long _creationTime;
		unsigned long _lastUpdateTime;
		unsigned long _timeout;
	public:
		UPnPCache();
		UPnPCache(unsigned long timeout);
		virtual ~UPnPCache();
		unsigned long & creationTime();
		unsigned long & lastUpdateTime();
		unsigned long & timeout();
		unsigned long lifetime();
		bool expired();
		void updateTime();
	};
}

#endif
