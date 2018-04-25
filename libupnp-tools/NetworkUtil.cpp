#include "NetworkUtil.hpp"
#include <vector>

namespace upnp {
    
    using namespace std;
    using namespace osl;
 
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
                    if (addr.inet4() && addr.getHost().compare("0.0.0.0")) {
                        return addr;
                    }
                }
            }
        }
        return InetAddress();
    }
    
}
