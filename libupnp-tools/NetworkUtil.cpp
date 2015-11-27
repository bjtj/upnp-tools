#include "NetworkUtil.hpp"
#include <vector>

namespace UPNP {
    
    using namespace std;
    using namespace OS;
 
    NetworkUtil::NetworkUtil() {
        
    }
    NetworkUtil::~NetworkUtil() {
        
    }
    InetAddress NetworkUtil::selectDefaultAddress() {
        vector<NetworkInterface> ifaces = Network::getNetworkInterfaces();
        for (size_t i = 0; i < ifaces.size(); i++) {
            NetworkInterface & iface = ifaces[i];
            if (!iface.isLoopBack()) {
                vector<InetAddress> addrs = iface.getInetAddresses();
                for (size_t i = 0; i < addrs.size(); i++) {
                    InetAddress & addr = addrs[i];
                    if (addr.inet4() && addr.getAddress().compare("0.0.0.0")) {
                        return addr;
                    }
                }
            }
        }
        return InetAddress();
    }
    
}