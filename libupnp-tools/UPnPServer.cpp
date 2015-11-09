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
    
    UrlSerializer::UrlSerializer() {
        
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
    
    /**
     *
     */

	UPnPServer::UPnPServer(int port) : actionRequestHandler(NULL), httpServer(port), pollingThread(NULL) {
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
        
        string st = header["ST"];
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
        
        logger.logd("onHttpRequest/path: " + request.getPath());
        
        if (!request.remaining()) {
            string path = request.getPath();
            if (Text::endsWith(path, "/device.xml")) {
                response.setContentType("text/xml");
                
                string udn = urlSerializer.getUdnFromUrlPath(path);
                udn = "uuid:" + udn;
                
                if (!udn.empty() && devices.hasDevice(udn)) {
                    string xml = getDeviceDescription(udn);
                    logger.logd(xml);
                    response.setContentLength((int)xml.length());
                    response.write(xml);
                } else {
                    response.setStatusCode(404, "No device found");
                    response.write("No device found...");
                }
            } else {
                response.setStatusCode(404);
            }
            
            response.setComplete();
        }
    }
    
    string UPnPServer::getDeviceDescription(const string & udn) {
        UPnPDeviceXmlWriter writer;
        UPnPDevice & device = devices.getDevice(udn);
        XmlDocument doc = writer.makeDeviceDescriptionXmlDocument(device);
        doc.setPrologue("?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        XmlPrinter printer;
        printer.setFormatted(true);
        return printer.printDocument(doc);
    }
    
    string UPnPServer::getScpd(const std::string & udn, const std::string scpdPath) {
        return "";
    }
    
    void UPnPServer::setActionRequestHandler(UPnPActionRequestHandler * actionRequestHandler) {
        this->actionRequestHandler = actionRequestHandler;
    }
	
}
