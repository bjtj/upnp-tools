#ifndef __NETWORK_STATE_MANAGER_HPP__
#define __NETWORK_STATE_MANAGER_HPP__

#include <string>
#include <vector>
#include <liboslayer/os.hpp>
#include <liboslayer/Observer.hpp>
#include <liboslayer/Network.hpp>

namespace upnp {
	
	class NetworkStateManager : public osl::Observable {
	private:
		std::string primaryInterface;
	public:
		NetworkStateManager();
		virtual ~NetworkStateManager();
		virtual std::vector<osl::InetAddress> getInetAddresses();
		virtual std::vector<osl::NetworkInterface> getNetworkInterfaces();
		void setPrimaryInterface(const std::string & primaryInterface);
		std::string getPirmaryInterface();
	};
}

#endif
