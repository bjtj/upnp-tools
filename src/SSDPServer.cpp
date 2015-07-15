#include "SSDPServer.hpp"
#include <algorithm>

namespace SSDP {

	using namespace std;
	using namespace OS;

	/**
	 * @brief polling thread (async running)
	 */
	PollingThread::PollingThread(SSDPServer & server) : server(server) {
	}
	PollingThread::~PollingThread() {
	}
	
	void PollingThread::run() {
		while (!interrupted()) {
			server.poll(100);
		}
	}

	/**
	 * @brief sddp server
	 */
	SSDPServer::SSDPServer() : socket(NULL), pollingThread(NULL) {
	}
	SSDPServer::~SSDPServer() {
	}

	void SSDPServer::start() {
		if (!socket) {
			socket = new DatagramSocket(1900);
			socket->setReuseAddr();
			socket->setBroadcast();
			socket->joinGroup("239.255.255.250");
			
			socket->registerSelector(selector);
		}
	}
	void SSDPServer::startAsync() {
		start();

		startPollingThread();
	}
	void SSDPServer::stop() {

		stopPollingThread();

		if (socket) {
			delete socket;
			socket = NULL;
		}
	}
	bool SSDPServer::isRunning() {
		return (socket != NULL);
	}

	void SSDPServer::poll(unsigned long timeout) {
		if (selector.select(timeout) > 0) {
			char buffer[4096] = {0,};
			if (socket->recv(buffer, sizeof(buffer)) > 0) {
				// handle message
			}
		}
	}

	void SSDPServer::startPollingThread() {
		if (!pollingThread) {
			pollingThread = new PollingThread(*this);
			pollingThread->start();
		}
	}
	void SSDPServer::stopPollingThread() {
		if (pollingThread) {
			pollingThread->interrupt();
			pollingThread->join();
			delete pollingThread;
			pollingThread = NULL;
		}
	}

	void SSDPServer::sendMsearch(string type) {
		string packet = "M-SEARCH * HTTP/1.1\r\n"
			"HOST: 239.255.255.250:1900\r\n"
			"MAN: \"ssdp:discovery\"\r\n"
			"MX: 3\r\n"
			"ST: " + type + "\r\n"
			"USER-AGENT: Linux/2.x UPnP/1.1 App/0.1\r\n"
			"\r\n";

		socket->send("239.255.255.250", 1900, (char*)packet.c_str(), packet.length());
	}

	void SSDPServer::addNotifyHandler(OnNotifyHandler * handler) {
		if (handler) {
			notifyHandlers.push_back(handler);
		}
	}
	void SSDPServer::removeNotifyHandler(OnNotifyHandler * handler) {
		if (handler) {
			notifyHandlers.erase(std::remove(notifyHandlers.begin(),
											 notifyHandlers.end(),
											 handler), notifyHandlers.end());
		}
	}
	void SSDPServer::addMsearchHandler(OnMsearchHandler * handler) {
		if (handler) {
			msearchHandlers.push_back(handler);
		}
	}
	void SSDPServer::removeMsearchHandler(OnMsearchHandler * handler) {
		if (handler) {
			msearchHandlers.erase(std::remove(msearchHandlers.begin(),
											 msearchHandlers.end(),
											 handler), msearchHandlers.end());
		}
	}
	
}
