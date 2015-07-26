#include "SSDPServer.hpp"
#include "Text.hpp"
#include <algorithm>

namespace SSDP {

	using namespace std;
	using namespace OS;
	using namespace HTTP;
	using namespace UTIL;

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
	 * @brief ssdp config
	 */
	SSDPConfig::SSDPConfig() :
		userAgent("Linux/2.x UPnP/1.1 App/0.1"),
		port(1900),
		multicastGroup("239.255.255.250") {
	}
	
	SSDPConfig::~SSDPConfig() {
	}

	void SSDPConfig::setUserAgent(std::string userAgent) {
		this->userAgent = userAgent;
	}

	std::string & SSDPConfig::getUserAgent() {
		return userAgent;
	}

	void SSDPConfig::setPort(int port) {
		this->port = port;
	}

	int SSDPConfig::getPort() {
		return port;
	}
	
	void SSDPConfig::setMulticastGroup(std::string group) {
		this->multicastGroup = group;
	}
	
	std::string & SSDPConfig::getMulticastGroup() {
		return multicastGroup;
	}

	/**
	 * @brief sddp server
	 */
	SSDPServer::SSDPServer() : socket(NULL), pollingThread(NULL) {
	}
	SSDPServer::SSDPServer(SSDPConfig & config) : config(config), socket(NULL), pollingThread(NULL) {
	}
	SSDPServer::~SSDPServer() {
	}

	void SSDPServer::start() {
		int port = config.getPort();
		string & group = config.getMulticastGroup();
		if (!socket) {
			socket = new DatagramSocket(port);
			socket->setReuseAddr();
			socket->setBroadcast();
			socket->joinGroup(group);
			
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
				handleMessage(buffer, sizeof(buffer));
			}
		}
	}

	void SSDPServer::handleMessage(const char * buffer, size_t size) {
		HttpHeaderParser parser;
		string header(buffer, size);

		int ret = parser.parse(header);
		if (ret == 0) {
			HttpRequestHeader header(parser.getHeader());
			string method = header.getMethod();
			if (!method.compare("M-SEARCH")) {
				onMsearch(header);
			} else if (!method.compare("NOTIFY")) {
				onNotify(header);
			} else {
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

	void SSDPServer::onMsearch(HttpHeader & header) {
		for (size_t i = 0; i < msearchHandlers.size(); i++) {
			msearchHandlers[i]->onMsearch(header);
		}
	}
	
	void SSDPServer::onNotify(HttpHeader & header) {
		for (size_t i = 0; i < notifyHandlers.size(); i++) {
			notifyHandlers[i]->onNotify(header);
		}
	}

	void SSDPServer::sendMsearch(string type) {

		int port = config.getPort();
		string & group = config.getMulticastGroup();
		string & userAgent = config.getUserAgent();

		string packet = "M-SEARCH * HTTP/1.1\r\n"
			"HOST: " + group +  ":" +  Text::toString(port) + "\r\n"
			"MAN: \"ssdp:discovery\"\r\n"
			"MX: 3\r\n"
			"ST: " + type + "\r\n"
			"USER-AGENT: " + userAgent + "\r\n"
			"\r\n";

		socket->send(group.c_str(), port, (char*)packet.c_str(), packet.length());
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

	SSDPConfig & SSDPServer::getConfig() {
		return config;
	}
	
}
