#ifndef __SSDP_HPP__
#define __SSDP_HPP__

#include <string>

namespace upnp {

	class SSDP
	{
	public:
		static std::string GROUP;
		static int PORT;
	private:
		SSDP();
		virtual ~SSDP();
	public:
		static std::string host();
	};
}

#endif
