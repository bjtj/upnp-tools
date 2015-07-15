#ifndef __SSDP_SERVER_HPP__
#define __SSDP_SERVER_HPP__

#include "os.hpp"

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

		virtual void onNotify() = 0;
	};

	/**
	 * @brief msearch handler
	 */
	class OnMsearchHandler {
	private:
	public:
		OnMsearchHandler() {}
		virtual ~OnMsearchHandler() {}

		virtual void onMsearch() = 0;
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
	 * @brief ssdep server
	 */
	class SSDPServer {
	private:
		std::vector<OnNotifyHandler*> notifyHandlers;
		std::vector<OnMsearchHandler*> msearchHandlers;
		OS::Selector selector;
		OS::DatagramSocket * socket;
		PollingThread * pollingThread;

	public:
		SSDPServer();
		virtual ~SSDPServer();

		virtual void start();
		virtual void startAsync();
		virtual void stop();
		virtual bool isRunning();
		virtual void poll(unsigned long timeout);

	private:
		void startPollingThread();
		void stopPollingThread();

	public:
		virtual void sendMsearch(std::string type);

		void addNotifyHandler(OnNotifyHandler * handler);
		void removeNotifyHandler(OnNotifyHandler * handler);
		void addMsearchHandler(OnMsearchHandler * handler);
		void removeMsearchHandler(OnMsearchHandler * handler);
	};
	
}

#endif
