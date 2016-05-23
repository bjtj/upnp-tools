#include "NetworkStateManager.hpp"

namespace UPNP {

	using namespace std;
	using namespace OS;
	
	NetworkStateManager::NetworkStateManager() {
	}
	
	NetworkStateManager::~NetworkStateManager() {
	}
	
	vector<InetAddress> NetworkStateManager::getInetAddresses() {
		vector<InetAddress> all;
		vector<NetworkInterface> ifaces = Network::getNetworkInterfaces();
        for (size_t i = 0; i < ifaces.size(); i++) {
			vector<InetAddress> addrs = ifaces[i].getInetAddresses();
			all.insert(all.end(), addrs.begin(), addrs.end());
        }
        return all;
	}

	vector<NetworkInterface> NetworkStateManager::getNetworkInterfaces() {
		return Network::getNetworkInterfaces();
	}

	void NetworkStateManager::setPrimaryInterface(const string & primaryInterface) {
		this->primaryInterface = primaryInterface;
	}
	
	string NetworkStateManager::getPirmaryInterface() {
		return primaryInterface;
	}
}
