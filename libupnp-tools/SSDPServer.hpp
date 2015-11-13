#ifndef __SSDP_SERVER_HPP__
#define __SSDP_SERVER_HPP__

#include <liboslayer/os.hpp>
#include <liboslayer/PollablePool.hpp>
#include <libhttp-server/HttpHeader.hpp>

#include <string>
#include <vector>

namespace SSDP {

	class SSDPServer;

	/**
	 * @brief notify handler
	 */
	class OnNotifyHandler {
	private:
	public:
		OnNotifyHandler() {}
		virtual ~OnNotifyHandler() {}

		virtual void onNotify(const HTTP::HttpHeader & header) = 0;
	};

	/**
	 * @brief msearch handler
	 */
	class OnMsearchHandler {
	private:
	public:
		OnMsearchHandler() {}
		virtual ~OnMsearchHandler() {}

        virtual void onMsearch(const HTTP::HttpHeader & header, const OS::InetAddress & remoteAddr) = 0;
	};

	/**
	 *
	 */
	class OnHttpResponseHandler {
	private:
	public:
		OnHttpResponseHandler() {}
		virtual ~OnHttpResponseHandler() {}

		virtual void onHttpResponse(const HTTP::HttpHeader & header) = 0;
	};
    
    /**
     * @brief Task
     */
    class Task {
    public:
        Task() {}
        virtual ~Task() {}
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual bool isRunning() = 0;
    };
    

	/**
	 * @brief ssdp configuration
	 */
	class SSDPConfig {
	private:
		std::string userAgent;
		int multicastPort;
		int msearchPort;
		OS::RandomPortBinder * msearchPortBinder;
		std::string multicastGroup;

	public:
		SSDPConfig();
		virtual ~SSDPConfig();

		void setUserAgent(std::string userAgent);
		std::string & getUserAgent();
		void setMulticastPort(int port);
		int getMulticastPort();
		void setMsearchPort(int msearchPort);
		int getMsearchPort();
		void setMsearchPortBinder(OS::RandomPortBinder * msearchPortBinder);
		OS::RandomPortBinder * getMsearchPortBinder();
		void setMulticastGroup(std::string group);
		std::string & getMulticastGroup();
	};
    
    
    /**
     * @brief ssdp listener
     */
    class SSDPListener : public UTIL::SelectablePollee, public Task {
    private:
        SSDPConfig config;
        std::vector<OnNotifyHandler*> notifyHandlers;
        std::vector<OnMsearchHandler*> msearchHandlers;
        OS::DatagramSocket * mcastSocket;
        
    public:
        SSDPListener();
        SSDPListener(SSDPConfig & config);
        virtual ~SSDPListener();
        
        virtual void start();
        virtual void stop();
        virtual bool isRunning();
        
        virtual void poll(unsigned long timeout);
        virtual void onIdle();
        virtual void listen(UTIL::SelectorPoller & poller);
        
        void handleMessage(const OS::DatagramPacket & packet);
        
        void onMsearch(const HTTP::HttpHeader & header, const OS::InetAddress & remoteAddr);
        void onNotify(const HTTP::HttpHeader & header);
        
        void addNotifyHandler(OnNotifyHandler * handler);
        void removeNotifyHandler(OnNotifyHandler * handler);
        void addMsearchHandler(OnMsearchHandler * handler);
        void removeMsearchHandler(OnMsearchHandler * handler);
        
        int sendMulticast(const char * data, size_t len);
    };
    
    /**
     * @brief MsearchSender
     */
    class MsearchSender : public UTIL::SelectablePollee, public Task {
    private:
        SSDPConfig config;
        std::vector<OnHttpResponseHandler*> httpResponseHandlers;
        OS::DatagramSocket * msearchSocket;
        
    public:
        MsearchSender();
        MsearchSender(SSDPConfig & config);
        virtual ~MsearchSender();
        
        virtual void start();
        virtual void stop();
        virtual bool isRunning();
        
        virtual void poll(unsigned long timeout);
        virtual void onIdle();
        virtual void listen(UTIL::SelectorPoller & poller);
                
        int sendMsearch(const std::string & type);
        
        void handleMessage(const OS::DatagramPacket & packet);
        
        void onHttpResponse(const HTTP::HttpHeader & header);
        
        void addHttpResponseHandler(OnHttpResponseHandler * handler);
        void removeHttpResponseHandler(OnHttpResponseHandler * handler);
    };

	
	/**
	 * @brief ssdep server
	 */
    class SSDPServer : public UTIL::SelectorPoller, public Task {
	private:
		SSDPConfig config;

        SSDPListener ssdpListener;
        MsearchSender msearchSender;
        
        UTIL::PollingThread * pollingThread;

	public:
		SSDPServer();
		SSDPServer(SSDPConfig & config);
		virtual ~SSDPServer();
        
    private:
        void init();
        
    public:

		virtual void start();
		virtual void startAsync();
		virtual void stop();
		virtual bool isRunning();
        
        void startPollingThread();
        void stopPollingThread();

		SSDPConfig & getConfig();
        
        int sendMsearch(const std::string & type);
        
        void addMsearchHandler(OnMsearchHandler * handler);
        void removeMsearchHandler(OnMsearchHandler * handler);
        void addNotifyHandler(OnNotifyHandler * handler);
        void removeNotifyHandler(OnNotifyHandler * handler);
        void addHttpResponseHandler(OnHttpResponseHandler * handler);
        void removeHttpResponseHandler(OnHttpResponseHandler * handler);
	};
	
}

#endif
