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
		unsigned long lifetimeRecent();
		unsigned long lifetimeFull();
		bool outdated();
		void prolong();
		void prolong(unsigned long newTimeout);
	};
}

#endif
