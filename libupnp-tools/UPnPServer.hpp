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
        
        virtual void onActionRequest(const UPnPActionRequest & request, UPnPActionResponse & response) = 0;
    };
    
    /**
     * @brief url serializer
     */
    class UrlSerializer {
    private:
		std::string prefix;
    public:
        UrlSerializer(const std::string & prefix);
        virtual ~UrlSerializer();
        
        virtual std::string makeDeviceDescriptionUrlPath(const UPnPDevice & device);
        virtual std::string getUdnFromUrlPath(const std::string & urlPath);
		virtual bool isDeviceDescriptionRequest(const std::string & urlPath);
		virtual std::string makeUrlPathPrefix(const std::string & udn);
		virtual std::string makeUrlPath(const std::string & udn, const std::string & append);
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
        
        void announceDeviceRecursive(const UPnPDevice & device);
        void announceDevice(const UPnPDevice & device);
        void announceService(const UPnPDevice & device, const UPnPService & service);
        void byebyeDeviceRecursive(const UPnPDevice & device);
        void byebyeDevice(const UPnPDevice & device);
        void byebyeService(const UPnPDevice & device, const UPnPService & service);
        
        virtual void onMsearch(const HTTP::HttpHeader & header, const OS::InetAddress & remoteAddr);
        HTTP::HttpHeader makeMsearchResponse(const UPnPDevice & device);
        HTTP::HttpHeader makeNotifyAlive(const UPnPDevice & device);
        HTTP::HttpHeader makeNotifyAlive(const UPnPDevice & device, const UPnPService & service);
        HTTP::HttpHeader makeNotifyAlive(const std::string & nt, const std::string usn, const UPnPDevice & device);
        HTTP::HttpHeader makeNotifyByebye(const UPnPDevice & device);
        HTTP::HttpHeader makeNotifyByebye(const UPnPDevice & device, const UPnPService & service);
        HTTP::HttpHeader makeNotifyByebye(const std::string & nt, const std::string usn, const UPnPDevice & device);
        std::string makeDeviceDescriptionUrl(const UPnPDevice & device);
        OS::InetAddress getHttpServerAddress();
        OS::InetAddress selectDefaultAddress();
        
        virtual void onHttpRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response);
		void onDeviceDescriptionRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response);
		void onScpdRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response, const UPnPService & service);
		void onControlRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response, const UPnPService & service);
		void onEventSubRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response, const UPnPService & service);

		std::string getUdnFromHttpRequest(HTTP::HttpRequest & request);
		std::string getActionNameFromHttpRequest(HTTP::HttpRequest & request);

        std::string getDeviceDescription(const std::string & udn);
        std::string getScpd(const Scpd & scpd);
        void setActionRequestHandler(UPnPActionRequestHandler * actionRequestHandler);

		UrlSerializer & getUrlSerializer();
	};
	
}

#endif
