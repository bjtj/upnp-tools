#ifndef __UPNP_SERVER_HPP__
#define __UPNP_SERVER_HPP__

#include <string>
#include <vector>

#include <liboslayer/PollablePool.hpp>
#include <libhttp-server/HttpServer.hpp>

#include "UPnPActionInvoke.hpp"
#include "SSDPServer.hpp"
#include "UPnPDevice.hpp"
#include "UPnPService.hpp"
#include "UPnPDevicePool.hpp"

namespace UPNP {

    /**
     * @brief
     */
    class UPnPActionRequestHandler {
    private:
    public:
        UPnPActionRequestHandler() {}
        virtual ~UPnPActionRequestHandler() {}
        
        virtual void onActionRequest(const UPnPActionRequest & request, const UPnPActionResponse & response) = 0;
    };
    
    /**
     * @brief url serializer
     */
    class UrlSerializer {
    private:
    public:
        UrlSerializer();
        virtual ~UrlSerializer();
        
        virtual std::string makeDeviceDescriptionUrlPath(const UPnPDevice & device);
        virtual std::string getUdnFromUrlPath(const std::string & urlPath);
    };
    
	/**
	 * @brief UPNP Server
	 */
    class UPnPServer : public SSDP::OnMsearchHandler, public HTTP::OnHttpRequestHandler {
	private:
        UPnPDevicePool devices;
        UPnPActionRequestHandler * actionRequestHandler;
        SSDP::SSDPListener ssdpListener;
        HTTP::HttpServer httpServer;
        UTIL::PollingThread * pollingThread;
        UrlSerializer urlSerializer;
		
	public:
		UPnPServer(int port);
		virtual ~UPnPServer();

		void start();
        void startAsync();
        void poll(unsigned long timeout);
        void stop();
		bool isRunning();
        
        UPnPDevice getDevice(const std::string & udn);
        void registerDeviceWithXml(const std::string & xmlDoc);
        void registerDevice(const UPnPDevice & device);
        void unregisterDevice(const std::string & udn);
        
        void announceDevice(const UPnPDevice & device);
        void announceDeviceRecursive(const UPnPDevice & device);
        void announceService(const UPnPService & service);
        void byebyeDevice(const UPnPDevice & device);
        void byebyeDeviceRecursive(const UPnPDevice & device);
        void byebyeService(const UPnPService & service);
        
        virtual void onMsearch(const HTTP::HttpHeader & header, const OS::InetAddress & remoteAddr);
        HTTP::HttpHeader makeMsearchResponse(const std::string & st, const UPnPDevice & device);
        HTTP::HttpHeader makeNofityAlive(const UPnPDevice & device);
        std::string makeDeviceDescriptionUrl(const UPnPDevice & device);
        OS::InetAddress getHttpServerAddress();
        OS::InetAddress selectDefaultAddress();
        
        virtual void onHttpRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response);
        std::string getDeviceDescription(const std::string & udn);
        std::string getScpd(const std::string & udn, const std::string scpdPath);
        void setActionRequestHandler(UPnPActionRequestHandler * actionRequestHandler);
	};
	
}

#endif
