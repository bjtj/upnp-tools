#include "SSDPServer.hpp"
#include <liboslayer/Text.hpp>
#include <liboslayer/RandomPortBinders.hpp>
#include <algorithm>
#include <libhttp-server/HttpHeaderParser.hpp>
#include "macros.hpp"
#include <liboslayer/Logger.hpp>

namespace SSDP {

	using namespace std;
	using namespace OS;
	using namespace HTTP;
	using namespace UTIL;

    static const Logger & logger = LoggerFactory::getDefaultLogger();

	/**
	 * @brief ssdp config
	 */
	SSDPConfig::SSDPConfig() :
		userAgent("Linux/2.x UPnP/1.1 App/0.1"),
		multicastPort(1900),
		msearchPort(56789),
		msearchPortBinder(NULL),
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

	void SSDPConfig::setMulticastPort(int multicastPort) {
		this->multicastPort = multicastPort;
	}

	int SSDPConfig::getMulticastPort() {
		return multicastPort;
	}

	void SSDPConfig::setMsearchPort(int msearchPort) {
		this->msearchPort = msearchPort;
	}

	int SSDPConfig::getMsearchPort() {
		return msearchPort;
	}

	void SSDPConfig::setMsearchPortBinder(RandomPortBinder * msearchPortBinder) {
		this->msearchPortBinder = msearchPortBinder;
	}

	RandomPortBinder * SSDPConfig::getMsearchPortBinder() {
		return msearchPortBinder;
	}
	
	void SSDPConfig::setMulticastGroup(std::string group) {
		this->multicastGroup = group;
	}
	
	std::string & SSDPConfig::getMulticastGroup() {
		return multicastGroup;
	}
    
    
    /**
     * @brief SSDPListener
     */
    
    SSDPListener::SSDPListener() : mcastSocket(NULL) {
        
    }
    
    SSDPListener::SSDPListener(SSDPConfig & config) : config(config), mcastSocket(NULL) {
        
    }
    
    SSDPListener::~SSDPListener() {
        
    }
    void SSDPListener::start() {
        int multicastPort = config.getMulticastPort();
        string & group = config.getMulticastGroup();
        if (!mcastSocket) {
            mcastSocket = new DatagramSocket(multicastPort);
            mcastSocket->setReuseAddr();
            mcastSocket->joinGroup(group);
            
            registerSelector(mcastSocket->getFd());
        }
    }
    void SSDPListener::stop() {
        notifyHandlers.clear();
        msearchHandlers.clear();
        if (mcastSocket) {

			unregisterSelector(mcastSocket->getFd());

            delete mcastSocket;
            mcastSocket = NULL;
        }
    }
    bool SSDPListener::isRunning() {
        return mcastSocket != NULL;
    }
    void SSDPListener::poll(unsigned long timeout) {
        getSelfSelectorPoller().poll(timeout);
    }
    
    void SSDPListener::listen(SelectorPoller & poller) {
        
        if (poller.isSelected(mcastSocket->getFd())) {
            char buffer[4096] = {0,};
            DatagramPacket packet(buffer, sizeof(buffer));
            int len;
            if ((len = mcastSocket->recv(packet)) > 0) {
                handleMessage(packet);
            }
        }
    }
    
    void SSDPListener::handleMessage(const DatagramPacket & packet) {
        HttpHeaderParser parser;
        string header(packet.getData(), packet.getLength());
        
        int ret = parser.parse(header);
        if (ret == 0) {
            HttpHeader & header = parser.getHeader();
            string method = header.getPart1();
            if (Text::equalsIgnoreCase(method, "M-SEARCH")) {
                onMsearch(header, InetAddress(packet.getRemoteAddr(), packet.getRemotePort()));
            } else if (Text::equalsIgnoreCase(method, "NOTIFY")) {
                onNotify(header);
            } else {
                // unknown ssdp message
            }
        } else {
            // unexpected message
        }
    }
    
    void SSDPListener::onMsearch(const HTTP::HttpHeader & header, const InetAddress & remoteAddr) {
        for (size_t i = 0; i < msearchHandlers.size(); i++) {
            msearchHandlers[i]->onMsearch(header, remoteAddr);
        }
    }
    
    void SSDPListener::onNotify(const HTTP::HttpHeader & header) {
        for (size_t i = 0; i < notifyHandlers.size(); i++) {
            notifyHandlers[i]->onNotify(header);
        }
    }
    
    void SSDPListener::addMsearchHandler(OnMsearchHandler * handler) {
        msearchHandlers.push_back(handler);
        
    }
    
    void SSDPListener::removeMsearchHandler(OnMsearchHandler * handler) {
        msearchHandlers.erase(std::remove(msearchHandlers.begin(),
                                          msearchHandlers.end(),
                                          handler), msearchHandlers.end());
    }
    
    void SSDPListener::addNotifyHandler(OnNotifyHandler * handler) {
        notifyHandlers.push_back(handler);
    }
    
    void SSDPListener::removeNotifyHandler(OnNotifyHandler * handler) {
        notifyHandlers.erase(std::remove(notifyHandlers.begin(),
                                         notifyHandlers.end(),
                                         handler), notifyHandlers.end());
    }
    
    int SSDPListener::sendMulticast(const char * data, size_t len) {
        if (mcastSocket) {
            string groupAddr = config.getMulticastGroup();
            int groupPort = config.getMulticastPort();
            return mcastSocket->send(groupAddr.c_str(), groupPort, data, len);
        }
        return -1;
    }
    
    /**
     * @brief MsearchSender
     */
    
    MsearchSender::MsearchSender() : msearchSocket(NULL) {
        
    }
    MsearchSender::MsearchSender(SSDPConfig & config) : config(config), msearchSocket(NULL) {
        
    }
    MsearchSender::~MsearchSender() {
        
    }
    
    void MsearchSender::start() {
        
        int msearchPort = config.getMsearchPort();
        
        if (!msearchSocket) {
            
            int bindResult;
            msearchSocket = new DatagramSocket(msearchPort);
            
            // https://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers
            // any random port range : 49152~65535
            /*class MyRandomPortBinder : public RandomPortBinder {
            private:
                int startPort;
                int currentPort;
                int endPort;
            public:
                MyRandomPortBinder() : startPort(49152), currentPort(0), endPort(65535) {}
                virtual ~MyRandomPortBinder() {}
                virtual void start() {
                    currentPort = startPort;
                }
                virtual int getNextPort() {
                    return currentPort++;
                }
                virtual bool wantFinish() {
                    return currentPort > endPort;
                }
                virtual int getSelectedPort() {
                    return currentPort;
                }
            };
            MyRandomPortBinder portBinder;
            bindResult = msearchSocket->randomBind(portBinder);*/

			RangeRandomPortBinder binder(49152, 65535);
			bindResult = msearchSocket->randomBind(binder);
            
            if (bindResult < 0) {
                throw IOException("bind() error", -1, 0);
            }
            
            registerSelector(msearchSocket->getFd());
        }

    }
    void MsearchSender::stop() {
        if (msearchSocket) {

			unregisterSelector(msearchSocket->getFd());

            delete msearchSocket;
            msearchSocket = NULL;
        }
    }
    bool MsearchSender::isRunning() {
        return msearchSocket != NULL;
    }
    void MsearchSender::poll(unsigned long timeout) {
        getSelfSelectorPoller().poll(timeout);
    }
    
    void MsearchSender::listen(SelectorPoller & poller) {
        
        if (poller.isSelected(msearchSocket->getFd())) {
            char buffer[4096] = {0,};
            DatagramPacket packet(buffer, sizeof(buffer));
            int len;
            if ((len = msearchSocket->recv(packet)) > 0) {
                handleMessage(packet);
            }
        }
    }
    
    int MsearchSender::sendMsearch(const string & type) {
        
        int multicastPort = config.getMulticastPort();
        string & group = config.getMulticastGroup();
        string & userAgent = config.getUserAgent();
        
        string packet = "M-SEARCH * HTTP/1.1\r\n"
        "HOST: " + group +  ":" +  Text::toString(multicastPort) + "\r\n"
        "MAN: \"ssdp:discover\"\r\n"
        "MX: 10\r\n"
        "ST: " + type + "\r\n"
        "USER-AGENT: " + userAgent + "\r\n"
        "Content-Length: 0\r\n"
        "\r\n";
        
        return  msearchSocket->send(group.c_str(), multicastPort, (char*)packet.c_str(), packet.length());
    }
    
    void MsearchSender::handleMessage(const DatagramPacket & packet) {
        HttpHeaderParser parser;
        string header(packet.getData(), packet.getLength());
        
        int ret = parser.parse(header);
        if (ret == 0) {
            HttpHeader & header = parser.getHeader();
            string method = header.getPart1();
            if (Text::startsWith(method, "HTTP")) {
                onHttpResponse(header);
            } else {
                // unknown ssdp message
            }
        } else {
            // unexpected message
        }
    }
    
    void MsearchSender::onHttpResponse(const HTTP::HttpHeader & header) {
        for (size_t i = 0; i < httpResponseHandlers.size(); i++) {
            httpResponseHandlers[i]->onHttpResponse(header);
        }
    }
    void MsearchSender::addHttpResponseHandler(OnHttpResponseHandler * handler) {
        httpResponseHandlers.push_back(handler);
    }
    void MsearchSender::removeHttpResponseHandler(OnHttpResponseHandler * handler) {
        httpResponseHandlers.erase(std::remove(httpResponseHandlers.begin(),
                                               httpResponseHandlers.end(),
                                               handler), httpResponseHandlers.end());
    }
    

	/**
	 * @brief sddp server
	 */
	SSDPServer::SSDPServer() : pollingThread(NULL) {
		init();
	}

	SSDPServer::SSDPServer(SSDPConfig & config) : config(config), ssdpListener(config), msearchSender(config), pollingThread(NULL) {
        init();
	}

	SSDPServer::~SSDPServer() {
        stop();
	}
    
    void SSDPServer::init() {
		registerSelectablePollee(&ssdpListener);
        registerSelectablePollee(&msearchSender);
    }

	void SSDPServer::start() {
        
        if (isRunning()) {
            return;
        }
        
        ssdpListener.start();
        msearchSender.start();
	}
	void SSDPServer::startAsync() {
        
        if (isRunning()) {
            return;
        }
        
		start();

		startPollingThread();
	}
	void SSDPServer::stop() {
        
        if (!isRunning()) {
            return;
        }

		stopPollingThread();
        
        ssdpListener.stop();
        msearchSender.stop();
	}
    
	bool SSDPServer::isRunning() {
        return ssdpListener.isRunning() && msearchSender.isRunning();
	}

	void SSDPServer::startPollingThread() {
		if (!pollingThread) {
			pollingThread = new PollingThread(*this, 1000);
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

	SSDPConfig & SSDPServer::getConfig() {
		return config;
	}
    
    int SSDPServer::sendMsearch(const string & type) {
        return msearchSender.sendMsearch(type);
    }
    
    void SSDPServer::addMsearchHandler(OnMsearchHandler * handler) {
        ssdpListener.addMsearchHandler(handler);
    }
    void SSDPServer::removeMsearchHandler(OnMsearchHandler * handler) {
        ssdpListener.removeMsearchHandler(handler);
    }
    void SSDPServer::addNotifyHandler(OnNotifyHandler * handler) {
        ssdpListener.addNotifyHandler(handler);
    }
    void SSDPServer::removeNotifyHandler(OnNotifyHandler * handler) {
        ssdpListener.removeNotifyHandler(handler);
    }
    void SSDPServer::addHttpResponseHandler(OnHttpResponseHandler * handler) {
        msearchSender.addHttpResponseHandler(handler);
    }
    void SSDPServer::removeHttpResponseHandler(OnHttpResponseHandler * handler) {
        msearchSender.removeHttpResponseHandler(handler);
    }
}
