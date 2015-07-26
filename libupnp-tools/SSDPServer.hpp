#ifndef __SSDP_SERVER_HPP__
#define __SSDP_SERVER_HPP__

#include "os.hpp"
#include "HttpHeader.hpp"

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

		virtual void onNotify(HTTP::HttpHeader & header) = 0;
	};

	/**
	 * @brief msearch handler
	 */
	class OnMsearchHandler {
	private:
	public:
		OnMsearchHandler() {}
		virtual ~OnMsearchHandler() {}

		virtual void onMsearch(HTTP::HttpHeader & header) = 0;
	};

	/**
	 * @brief polling thread
	 */
	class PollingThread : public OS::Thread {
	private:
		SSDPServer & server;
	public:
		PollingThread(SSDPServer & server);
		virtual ~PollingThread();

		virtual void run();
	};

	/**
	 * @brief ssdp configuration
	 */
	class SSDPConfig {
	private:
		std::string userAgent;
		int port;
		std::string multicastGroup;
	public:
		SSDPConfig();
		virtual ~SSDPConfig();

		void setUserAgent(std::string userAgent);
		std::string & getUserAgent();
		void setPort(int port);
		int getPort();
		void setMulticastGroup(std::string group);
		std::string & getMulticastGroup();
	};

	
	/**
	 * @brief ssdep server
	 */
	class SSDPServer {
	private:
		SSDPConfig config;
		std::vector<OnNotifyHandler*> notifyHandlers;
		std::vector<OnMsearchHandler*> msearchHandlers;
		OS::Selector selector;
		OS::DatagramSocket * socket;
		PollingThread * pollingThread;

	public:
		SSDPServer();
		SSDPServer(SSDPConfig & config);
		virtual ~SSDPServer();

		virtual void start();
		virtual void startAsync();
		virtual void stop();
		virtual bool isRunning();
		virtual void poll(unsigned long timeout);

		void handleMessage(const char * buffer, size_t size);

	private:
		void startPollingThread();
		void stopPollingThread();
		void onMsearch(HTTP::HttpHeader & header);
		void onNotify(HTTP::HttpHeader & header);

	public:
		virtual void sendMsearch(std::string type);

		void addNotifyHandler(OnNotifyHandler * handler);
		void removeNotifyHandler(OnNotifyHandler * handler);
		void addMsearchHandler(OnMsearchHandler * handler);
		void removeMsearchHandler(OnMsearchHandler * handler);

		SSDPConfig & getConfig();
	};
	
}

#endif
