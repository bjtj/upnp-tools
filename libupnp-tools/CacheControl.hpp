#ifndef __CACHE_CONTROL_HPP__
#define __CACHE_CONTROL_HPP__

namespace UPNP {


	class CacheControl {
	private:
		unsigned long createdTick;
		unsigned long timeoutMilli;

	public:
		CacheControl();
		virtual ~CacheControl();
		void renew();
		void setTimeout(unsigned long timeoutMilli);
		bool outdated() const;
	};
}

#endif