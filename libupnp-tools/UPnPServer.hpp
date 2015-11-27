#ifndef __UPNP_SERVER_HPP__
#define __UPNP_SERVER_HPP__

#include <string>
#include <vector>

#include <liboslayer/PollablePool.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <libhttp-server/AnotherHttpClientThreadPool.hpp>

#include "UPnPActionInvoke.hpp"
#include "SSDPServer.hpp"
#include "UPnPDevice.hpp"
#include "UPnPService.hpp"
#include "UPnPDevicePool.hpp"
#include "Timer.hpp"
#include "NetworkUtil.hpp"

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
     * @brief
     */
    class UPnPEventSubscribeInfo {
    private:
        std::string sid;
        UPnPService service;
        std::vector<std::string> callbacks;
        unsigned int seq;
        
    public:
        UPnPEventSubscribeInfo();
        UPnPEventSubscribeInfo(const std::string & sid, UPnPService & service, std::vector<std::string> & callbacks);
        virtual ~UPnPEventSubscribeInfo();
        
        bool empty();
        
        std::string getSid();
        std::vector<std::string> & getCallbacks();
        UPnPService & getService();
        
        void initSeq();
        unsigned int nextSeq();
        unsigned int getSeq();
    };
    
    /**
     * @brief
     */
    class UPnPEventSubscriberPool {
    private:
        std::map<std::string, UPnPEventSubscribeInfo> subscribers;
    public:
        UPnPEventSubscriberPool();
        virtual ~UPnPEventSubscriberPool();
        
        std::string registerSubscriber(UPnPService & service, std::vector<std::string> & callbackUrls);
        void unregisterSubscriber(const std::string & sid);
        UPnPEventSubscribeInfo getSubscriberInfo(const std::string & sid);
        UPnPEventSubscribeInfo getSubscriberInfo(UPnPService & service);
        
        std::string generateSid();
    };
    
    /**
     * @brief UPnPEventSubscribeListener
     */
    class UPnPEventSubscribeListener {
    private:
    public:
        UPnPEventSubscribeListener() {}
        virtual ~UPnPEventSubscribeListener() {}
        
        virtual void onEventSubsribe(UPnPService & service) = 0;
    };
    
	/**
	 * @brief UPNP Server
	 */
    class UPnPServer : public SSDP::OnMsearchHandler, public HTTP::HttpRequestHandler, public UTIL::SelectorPoller, public TimerEvent {
	private:
        UPnPDevicePool devicePool;
        UPnPActionRequestHandler * actionRequestHandler;
        SSDP::SSDPListener ssdpListener;
        //HTTP::HttpServer httpServer;
		HTTP::AnotherHttpServer httpServer;
        UTIL::PollingThread * pollingThread;
        UrlSerializer urlSerializer;
        Timer timer;
        int idx;
        
        UPnPEventSubscriberPool subsriberPool;
        UPnPEventSubscribeListener * eventSubscribeListener;
        HTTP::AnotherHttpClientThreadPool httpClientThreadPool;
		
	public:
		UPnPServer(int port);
		virtual ~UPnPServer();
        
        virtual void onTimerTriggered();

		void start();
        void startAsync();
        void stop();
		bool isRunning();
        virtual void poll(unsigned long timeout);
        
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
        
        virtual void onHttpRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response);
		virtual void onHttpRequestContent(HTTP::HttpRequest & request, HTTP::HttpResponse &response, HTTP::Packet & packet);
        virtual void onHttpRequestContentCompleted(HTTP::HttpRequest &request, HTTP::HttpResponse &response);
		void onDeviceDescriptionRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response);
		void onScpdRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response, UPnPService & service);
		void onControlRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response, UPnPService & service);
		void onEventSubRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response, UPnPService & service);
        
        std::vector<std::string> parseCallbackUrls(const std::string & callbackPhrase);
        void noitfyPropertyChanged(UPnPService & service, UTIL::LinkedStringMap & values);

		std::string getUdnFromHttpRequest(HTTP::HttpRequest & request);
		std::string getActionNameFromHttpRequest(HTTP::HttpRequest & request);

        std::string getDeviceDescription(const std::string & udn);
        std::string getScpd(const Scpd & scpd);
        void setActionRequestHandler(UPnPActionRequestHandler * actionRequestHandler);
        
        void sendHttpRequest(const HTTP::Url & url, const std::string & method, const UTIL::StringMap & additionalFields, const std::string & content, HTTP::UserData * userData);
        virtual void onRequestComplete(HTTP::Url & url, HTTP::HttpResponse & response, const std::string & content, HTTP::UserData * userData);
        virtual void onRequestError(OS::Exception & e, HTTP::Url & url, HTTP::UserData * userData);
        
        std::string toPropertySetXmlString(UTIL::LinkedStringMap props);

		UrlSerializer & getUrlSerializer();
        void setEventSubscribeListener(UPnPEventSubscribeListener * eventSubscribeListener);
	};
	
}

#endif
