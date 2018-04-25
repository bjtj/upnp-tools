#ifndef __NETWORK_UTIL_HPP__
#define __NETWORK_UTIL_HPP__

#include <liboslayer/os.hpp>
#include <liboslayer/Network.hpp>

namespace upnp {
    
    class NetworkUtil {
    private:
    public:
        NetworkUtil();
        virtual ~NetworkUtil();
        static osl::InetAddress selectDefaultAddress();
    };
    
}

#endif
