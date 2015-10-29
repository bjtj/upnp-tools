#include "SSDPServer.hpp"
#include <liboslayer/Text.hpp>
#include <algorithm>
#include <libhttp-server/HttpHeaderParser.hpp>

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
		msearchPort(12345),
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

	void SSDPConfig::setMsearchPort(int msearchPort) {
		this->msearchPort = port;
	}

	int SSDPConfig::getMsearchPort() {
		return msearchPort;
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
	SSDPServer::SSDPServer() : mcastSocket(NULL), msearchSocket(NULL), pollingThread(NULL) {
	}
	SSDPServer::SSDPServer(SSDPConfig & config) : config(config), mcastSocket(NULL), msearchSocket(NULL), pollingThread(NULL) {
	}
	SSDPServer::~SSDPServer() {
	}

	void SSDPServer::start() {
		int port = config.getPort();
		int msearchPort = config.getMsearchPort();
		string & group = config.getMulticastGroup();
		if (!mcastSocket) {
			mcastSocket = new DatagramSocket(port);
			mcastSocket->setReuseAddr();
			mcastSocket->joinGroup(group);
			
			mcastSocket->registerSelector(selector);
		}

		if (!msearchSocket) {

			msearchSocket = new DatagramSocket(msearchPort);
			msearchSocket->setReuseAddr();
			msearchSocket->bind();
			
			msearchSocket->registerSelector(selector);
		}
	}
	void SSDPServer::startAsync() {
		start();

		startPollingThread();
	}
	void SSDPServer::stop() {

		stopPollingThread();

		notifyHandlers.clear();
		msearchHandlers.clear();
		httpResponseHandlers.clear();

		if (socket) {
			delete mcastSocket;
			mcastSocket = NULL;
		}

		if (msearchSocket) {
			delete msearchSocket;
			msearchSocket = NULL;
		}
	}
	bool SSDPServer::isRunning() {
		return (socket != NULL);
	}

	void SSDPServer::poll(unsigned long timeout) {
		if (selector.select(timeout) > 0) {
			if (selector.isSelected(mcastSocket->getFd())) {
				char buffer[4096] = {0,};
				DatagramPacket packet(buffer, sizeof(buffer));
				int len;
				if ((len = mcastSocket->recv(packet)) > 0) {
					handleMessage(packet.getData(), packet.getLength());
				}
			}

			if (selector.isSelected(msearchSocket->getFd())) {
				char buffer[4096] = {0,};
				DatagramPacket packet(buffer, sizeof(buffer));
				int len;
				if ((len = msearchSocket->recv(packet)) > 0) {
					handleMessage(packet.getData(), packet.getLength());
				}
			}
		}
	}

	void SSDPServer::handleMessage(const char * buffer, size_t size) {
		HttpHeaderParser parser;
		string header(buffer, size);

		int ret = parser.parse(header);
		if (ret == 0) {
			HttpHeader & header = parser.getHeader();
			string method = header.getPart1();
			if (Text::equalsIgnoreCase(method, "M-SEARCH")) {
				onMsearch(header);
			} else if (Text::equalsIgnoreCase(method, "NOTIFY")) {
				onNotify(header);
			} else if (Text::startsWith(method, "HTTP")) {
				onHttpResponse(header);
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

	void SSDPServer::onHttpResponse(HttpHeader & header) {
		for (size_t i = 0; i < httpResponseHandlers.size(); i++) {
			httpResponseHandlers[i]->onHttpResponse(header);
		}
	}

	int SSDPServer::sendMsearch(string type) {

		int port = config.getPort();
		string & group = config.getMulticastGroup();
		string & userAgent = config.getUserAgent();

		string packet = "M-SEARCH * HTTP/1.1\r\n"
			"HOST: " + group +  ":" +  Text::toString(port) + "\r\n"
			"MAN: \"ssdp:discover\"\r\n"
			"MX: 10\r\n"
			"ST: " + type + "\r\n"
			"USER-AGENT: " + userAgent + "\r\n"
			"Content-Length: 0\r\n"
			"\r\n";

		return  msearchSocket->send(group.c_str(), port, (char*)packet.c_str(), packet.length());
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

	void SSDPServer::addHttpResponseHandler(OnHttpResponseHandler * handler) {
		if (handler) {
			httpResponseHandlers.push_back(handler);
		}
	}
	void SSDPServer::removeHttpResponseHandler(OnHttpResponseHandler * handler) {
		if (handler) {
			httpResponseHandlers.erase(std::remove(httpResponseHandlers.begin(),
											 httpResponseHandlers.end(),
											 handler), httpResponseHandlers.end());
		}
	}

	SSDPConfig & SSDPServer::getConfig() {
		return config;
	}
	
}
