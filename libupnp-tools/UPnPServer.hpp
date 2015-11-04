#ifndef __UPNP_SERVER_HPP__
#define __UPNP_SERVER_HPP__

#include <string>
#include <vector>

#include <libhttp-server/HttpServer.hpp>

#include "SSDPServer.hpp"
#include "UPnPDevice.hpp"
#include "UPnPService.hpp"
#include "UPnPDevicePool.hpp"

namespace UPNP {

    class InvokeActionListener {
    private:
    public:
        InvokeActionListener() {}
        virtual ~InvokeActionListener() {}
        
        virtual void onActionRequest() = 0;
    };

	/**
	 * @brief UPNP Server
	 */
	class UPnPServer {
	private:
        UPnPDevicePool devices;
		SSDP::SSDPServer ssdpServer;
        InvokeActionListener * actionListener;
		
	public:
		UPnPServer();
		virtual ~UPnPServer();

		void start();
        void startAsync();
        void poll(unsigned long timeout);
        void stop();
		bool isRunning();
        
        void registerDevice(const UPnPDevice & device);
        void unregisterDevice(const std::string & udn);
        
        void announceDevice(const UPnPDevice & device);
        void announceDeviceRecursive(const UPnPDevice & device);
        void announceService(const UPnPService & service);
        void byebyeDevice(const UPnPDevice & device);
        void byebyeDeviceRecursive(const UPnPDevice & device);
        void byebyeService(const UPnPService & service);
        
        void setInvokeActionListener(InvokeActionListener * actionListener);
	};
	
}

#endif
