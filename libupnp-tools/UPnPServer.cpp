#include "UPnPServer.hpp"
#include "Uuid.hpp"
#include <liboslayer/Text.hpp>
#include <liboslayer/Logger.hpp>
#include "XmlDomParser.hpp"
#include "UPnPDeviceMaker.hpp"
#include "UPnPDeviceXmlWriter.hpp"
#include "XmlDocumentPrinter.hpp"

namespace UPNP {

    using namespace std;
	using namespace SSDP;
    using namespace UTIL;
    using namespace HTTP;
    using namespace OS;
    using namespace XML;
    
    static const Logger & logger = LoggerFactory::getDefaultLogger();
    
    /**
     *
     */
    
    UrlSerializer::UrlSerializer(const string & prefix) : prefix(prefix) {
        
    }
    UrlSerializer::~UrlSerializer() {
        
    }
    
    string UrlSerializer::makeDeviceDescriptionUrlPath(const UPnPDevice & device) {
        Uuid uuid(device.getUdn());
        string uuidPart = uuid.getUuid();
        if (Text::startsWith(uuidPart, "uuid:")) {
            return "/" + uuidPart.substr(5) + "/device.xml";
        }
        return "";
    }
    string UrlSerializer::getUdnFromUrlPath(const std::string & urlPath) {
        size_t f = urlPath.find_first_not_of("/");
        if (f == string::npos) {
            return "";
        }
        
        size_t e = urlPath.find("/", f);
        if (e == string::npos) {
            return "";
        }
        
        return urlPath.substr(f, e - f);
    }

	bool UrlSerializer::isDeviceDescriptionRequest(const string & urlPath) {
		return Text::endsWith(urlPath, "/device.xml");
	}

	string UrlSerializer::makeUrlPathPrefix(const string & udn) {
		string udnWithoutUuidPart = udn;
		if (Text::startsWith(udnWithoutUuidPart, "uuid:")) {
			udnWithoutUuidPart = udnWithoutUuidPart.substr(5);
		}
		return prefix + (!Text::endsWith(prefix, "/") ? "/" : "") + udnWithoutUuidPart + "/";
	}

	string UrlSerializer::makeUrlPath(const string & udn, const string & append) {
		string prefix = makeUrlPathPrefix(udn);
		string appendWithoutSlash = append;
		if (Text::startsWith(appendWithoutSlash, "/")) {
			appendWithoutSlash = appendWithoutSlash.substr(1);
		}
		return prefix + (!Text::endsWith(prefix, "/") ? "/" : "") + appendWithoutSlash;
	}
    /**
     *
     */

	UPnPServer::UPnPServer(int port) : actionRequestHandler(NULL), httpServer(port), pollingThread(NULL), urlSerializer("") {
        httpServer.vpath("/*", this);
        ssdpListener.addMsearchHandler(this);
	}
	
	UPnPServer::~UPnPServer() {
        stop();
	}
	
	void UPnPServer::start() {
        ssdpListener.start();
        httpServer.start();
	}
    
    void UPnPServer::startAsync() {
        
        start();
        
        httpServer.startAsync();
        
        if (!pollingThread) {
            pollingThread = new PollingThread(ssdpListener.getSelfSelectorPoller(), 1000);
            pollingThread->start();
        }
    }
    
    void UPnPServer::poll(unsigned long timeout) {
        ssdpListener.poll(timeout);
    }
	
	void UPnPServer::stop() {
        
        httpServer.stop();
        
        if (pollingThread) {
            pollingThread->interrupt();
            pollingThread->join();
            delete pollingThread;
            pollingThread = NULL;
        }
        
        ssdpListener.stop();
	}
	
	bool UPnPServer::isRunning() {
        return ssdpListener.isRunning();
	}
    
    UPnPDevice UPnPServer::getDevice(const string & udn) {
        return devices.getDevice(udn);
    }
    
    void UPnPServer::registerDeviceWithXml(const string & xmlDoc) {
        XmlDomParser parser;
        XmlDocument dom = parser.parse(xmlDoc);
        UPnPDevice device = UPnPDeviceMaker::makeDeviceFromDeviceDescription("file://./", dom);
        registerDevice(device);
    }
    
    void UPnPServer::registerDevice(const UPnPDevice & device) {
        announceDeviceRecursive(device);
        devices.addDevice(device);
    }
    void UPnPServer::unregisterDevice(const std::string & udn) {
        if (devices.hasDevice(udn)) {
            UPnPDevice & device = devices.getDevice(udn);
            byebyeDeviceRecursive(device);
        }
    }
    
    void UPnPServer::announceDevice(const UPnPDevice & device) {
        DatagramSocket socket("239.255.255.250", 1900);
        InetAddress address("127.0.0.1", 8080);
        HttpHeader responseHeader = makeNofityAlive(device);
        string packet = responseHeader.toString();
        socket.send("239.255.255.250", 1900, packet.c_str(), packet.length());
    }
    void UPnPServer::announceDeviceRecursive(const UPnPDevice & device) {
        
    }
    void UPnPServer::announceService(const UPnPService & service) {
        
    }
    
    void UPnPServer::byebyeDevice(const UPnPDevice & device) {
        
    }
    void UPnPServer::byebyeDeviceRecursive(const UPnPDevice & device) {
        
    }
    void UPnPServer::byebyeService(const UPnPService & service) {
        
    }
    
    void UPnPServer::onMsearch(const HttpHeader & header, const InetAddress & remoteAddr) {
        
        string st = header.getHeaderFieldIgnoreCase("ST");
        if (Text::equalsIgnoreCase(st, "upnp:rootdevice")) {
            
            vector<UPnPDevice> roots = devices.getRootDevices();
            
            for (size_t i = 0; i < roots.size(); i++) {
                UPnPDevice device = roots[i];
                DatagramSocket socket(remoteAddr.getAddress().c_str(), remoteAddr.getPort());
                HttpHeader responseHeader = makeMsearchResponse(st, device);
                string packet = responseHeader.toString();
				// logger.logd(packet);
                socket.send(remoteAddr.getAddress().c_str(), remoteAddr.getPort(), packet.c_str(), packet.length());
            }
        }
    }
    
    HttpHeader UPnPServer::makeMsearchResponse(const string & st, const UPnPDevice & device) {
        Uuid uuid(device.getUdn());
        if (device.isRootDevice()) {
            uuid.setRest("upnp:rootdevice");
        } else {
            uuid.setRest(device.getDeviceType());
        }
        
        string deviceDescriptionLocation = makeDeviceDescriptionUrl(device);
        
        HttpHeader responseHeader;
        responseHeader.setContentLength(0);
        responseHeader.setParts("HTTP/1.1", "200", "OK");
        responseHeader.setHeaderField("CACHE-CONTROL", "max-age=1800");
        responseHeader.setHeaderField("ST", st);
        responseHeader.setHeaderField("USN", uuid.toString());
        responseHeader.setHeaderField("EXT", "");
        responseHeader.setHeaderField("SERVER", "Platform/0.1 UPnP/0.1 App/0.1");
        responseHeader.setHeaderField("LOCATION", deviceDescriptionLocation);
        return responseHeader;
    }
    
    HttpHeader UPnPServer::makeNofityAlive(const UPnPDevice & device) {
        
        Uuid uuid(device.getUdn());
        if (device.isRootDevice()) {
            uuid.setRest("upnp:rootdevice");
        } else {
            uuid.setRest(device.getDeviceType());
        }
        
        string deviceDescriptionLocation = makeDeviceDescriptionUrl(device);
        
        HttpHeader responseHeader;
        responseHeader.setContentLength(0);
        responseHeader.setParts("NOTIFY", "*", "HTTP/1.1");
        responseHeader.setHeaderField("HOST", "239.255.255.250:1900");
        responseHeader.setHeaderField("CACHE-CONTROL", "max-age=1800");
        responseHeader.setHeaderField("NT", "upnp:rootdevice");
        responseHeader.setHeaderField("USN", uuid.toString());
        responseHeader.setHeaderField("NTS", "ssdp:alive");
        responseHeader.setHeaderField("SERVER", "Platform/0.1 UPnP/0.1 App/0.1");
        responseHeader.setHeaderField("LOCATION", deviceDescriptionLocation);
        return responseHeader;
    }
    
    string UPnPServer::makeDeviceDescriptionUrl(const UPnPDevice & device) {
        
        InetAddress addr = getHttpServerAddress();
        
        string path = urlSerializer.makeDeviceDescriptionUrlPath(device);
        string url = "http://" + addr.getAddress() + ":" + Text::toString(addr.getPort()) + path;
        return url;
    }
    
    InetAddress UPnPServer::getHttpServerAddress() {
        InetAddress addr = selectDefaultAddress();
        addr.setPort(httpServer.getPort());
        return addr;
    }
    
    InetAddress UPnPServer::selectDefaultAddress() {
        vector<NetworkInterface> ifaces = Network::getNetworkInterfaces();
        for (size_t i = 0; i < ifaces.size(); i++) {
            NetworkInterface & iface = ifaces[i];
            if (!iface.isLoopBack()) {
                vector<InetAddress> addrs = iface.getInetAddresses();
                for (size_t i = 0; i < addrs.size(); i++) {
					InetAddress & addr = addrs[i];
					if (addr.inet4() && addr.getAddress().compare("0.0.0.0")) {
                        return addr;
                    }
                }
            }
        }
        return InetAddress();
    }

    void UPnPServer::onHttpRequest(HttpRequest & request, HttpResponse & response) {
        
        
        // TODO: make clean this
        int len = request.getContentLength();
        ChunkedBuffer & buffer = request.getChunkedBuffer();
        if (len > 0) {
            
            if (len != buffer.getChunkSize()) {
                buffer.setChunkSize(len);
            }
            
            char readBuffer[1024] = {0,};
            int readLen = request.getSocket().recv(readBuffer, buffer.getReadSize(sizeof(readBuffer)));
            buffer.readChunkData(readBuffer, readLen);
        }
        
        if (buffer.remainingDataBuffer() == 0) {
            
            logger.logd("onHttpRequest/path: " + request.getPath());
            
            string path = request.getPath();

            if (urlSerializer.isDeviceDescriptionRequest(path)) {
                onDeviceDescriptionRequest(request, response);
            } else {

				string udn = urlSerializer.getUdnFromUrlPath(path);
				if (!udn.empty()) {

					udn = "uuid:" + udn;
					if (devices.hasDevice(udn)) {
						UPnPDevice device = devices.getDevice(udn);
						if (device.hasServiceWithPropertyRecursive("SCPDURL", path)) {
							onScpdRequest(request, response, device.getServiceWithPropertyRecursive("SCPDURL", path));
						} else if (device.hasServiceWithPropertyRecursive("controlURL", path)) {
							onControlRequest(request, response, device.getServiceWithPropertyRecursive("controlURL", path));
						} else if (device.hasServiceWithPropertyRecursive("eventSubURL", path)) {
							onEventSubRequest(request, response, device.getServiceWithPropertyRecursive("eventSubURL", path));
						} else {
							response.setStatusCode(404);
						}

					} else {
						response.setStatusCode(404);
					}

				} else {
					response.setStatusCode(404);
				}
            }
            
            response.setComplete();
        }
    }

	void UPnPServer::onDeviceDescriptionRequest(HttpRequest & request, HttpResponse & response) {

        string udn = getUdnFromHttpRequest(request);
                
        if (devices.hasDevice(udn)) {
            string xml = getDeviceDescription(udn);

            logger.logd(xml);

			response.setStatusCode(200);
			response.setContentType("text/xml");
            response.setContentLength((int)xml.length());
            response.write(xml);

        } else {
            response.setStatusCode(404, "No device found");
            response.write("No device found...");
        }

	}
	void UPnPServer::onScpdRequest(HttpRequest & request, HttpResponse & response, const UPnPService & service) {

		string xml = getScpd(service.getScpd());

		logger.logd(xml);

		response.setStatusCode(200);
		response.setContentType("text/xml");
        response.setContentLength((int)xml.length());
        response.write(xml);
	}
	void UPnPServer::onControlRequest(HttpRequest & request, HttpResponse & response, const UPnPService & service) {
		
		int len = request.getContentLength();
        ChunkedBuffer buffer = request.getChunkedBuffer();
        string content(buffer.getChunkData(), buffer.getChunkSize());

		logger.logd("(" + Text::toString(len) + ")" + content);
		
		response.setStatusCode(500, "Not supported yet");

		if (actionRequestHandler) {
			//actionRequestHandler->onActionRequest(actionRequest, actionResponse);
		}
	}
	void UPnPServer::onEventSubRequest(HttpRequest & request, HttpResponse & response, const UPnPService & service) {
		response.setStatusCode(500, "Not supported yet");
	}
    
	string UPnPServer::getUdnFromHttpRequest(HttpRequest & request) {
		string path = request.getPath();
        string udn = urlSerializer.getUdnFromUrlPath(path);
        udn = "uuid:" + udn;
		return udn;
	}

	string UPnPServer::getActionNameFromHttpRequest(HttpRequest & request) {
		return "";
	}

    string UPnPServer::getDeviceDescription(const string & udn) {
        UPnPDeviceXmlWriter writer;
        UPnPDevice & device = devices.getDevice(udn);
        XmlDocument doc = writer.makeDeviceDescriptionXmlDocument(device);
        doc.setPrologue("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        XmlPrinter printer;
		printer.setShowPrologue(true);
        printer.setFormatted(true);
        return printer.printDocument(doc);
    }
    
    string UPnPServer::getScpd(const Scpd & scpd) {
		ScpdXmlWriter writer;
		XmlDocument doc = writer.makeScpdXmlDocument(scpd);
		doc.setPrologue("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        XmlPrinter printer;
		printer.setShowPrologue(true);
        printer.setFormatted(true);
        return printer.printDocument(doc);
    }
    
    void UPnPServer::setActionRequestHandler(UPnPActionRequestHandler * actionRequestHandler) {
        this->actionRequestHandler = actionRequestHandler;
    }
	
	UrlSerializer & UPnPServer::getUrlSerializer() {
		return urlSerializer;
	}
}
