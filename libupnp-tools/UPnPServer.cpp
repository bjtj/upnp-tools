#include "UPnPServer.hpp"
#include "Uuid.hpp"
#include <liboslayer/Text.hpp>
#include <liboslayer/Logger.hpp>
#include "XmlDomParser.hpp"
#include "UPnPDeviceMaker.hpp"
#include "UPnPDeviceXmlWriter.hpp"
#include "XmlDocumentPrinter.hpp"
#include "SoapWriter.hpp"
#include "SoapReader.hpp"

namespace UPNP {

    using namespace std;
	using namespace SSDP;
    using namespace UTIL;
    using namespace HTTP;
    using namespace OS;
    using namespace XML;
	using namespace SOAP;
    
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
     * @brief UPnPServer
     */

	UPnPServer::UPnPServer(int port) : TimerEvent(false), actionRequestHandler(NULL), httpServer(port), pollingThread(NULL), urlSerializer(""), idx(0) {
        
        registerPollee(&timer);
        registerSelectablePollee(&ssdpListener);
        
        // httpServer.vpath("/*", this);
		httpServer.registerRequestHandler("/*", this);

        ssdpListener.addMsearchHandler(this);
	}
	
	UPnPServer::~UPnPServer() {
        stop();
	}
    
    void UPnPServer::onFire() {
//        logger.logd("..." + Text::toString(idx++));
    }
	
	void UPnPServer::start() {
        ssdpListener.start();
        httpServer.start();
        timer.start();
        
        scheduleRepeatableRelativeTimer(0, -1, Timer::SECOND);
        timer.setTimerEvent(this);
	}
    
    void UPnPServer::startAsync() {
        
        start();
        
        httpServer.startAsync();
        
        if (!pollingThread) {
            pollingThread = new PollingThread(this, 1000);
            pollingThread->start();
        }
    }
	
	void UPnPServer::stop() {
        
        timer.stop();
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
        if (!devices.hasDevice(device.getUdn())) {
            devices.addDevice(device);
            announceDeviceRecursive(device);
        }
    }
    void UPnPServer::unregisterDevice(const std::string & udn) {
        if (devices.hasDevice(udn)) {
            UPnPDevice & device = devices.getDevice(udn);
            byebyeDeviceRecursive(device);
            devices.removeDevice(udn);
        }
    }
    void UPnPServer::announceDeviceRecursive(const UPnPDevice & device) {
        announceDevice(device);
        const vector<UPnPDevice> & embeds = device.getEmbeddedDevices();
        for (size_t i = 0; i < embeds.size(); i++) {
            const UPnPDevice & embed = embeds[i];
            announceDevice(embed);
        }
        const vector<UPnPService> & services = device.getServices();
        for (size_t i = 0; i < services.size(); i++) {
            const UPnPService & service = services[i];
            announceService(device, service);
        }
    }
    void UPnPServer::announceDevice(const UPnPDevice & device) {
        HttpHeader notifyHeader = makeNotifyAlive(device);
        string packet = notifyHeader.toString();
        ssdpListener.sendMulticast(packet.c_str(), packet.length());
        
        if (device.isRootDevice()) {
            notifyHeader = makeNotifyAlive(device.getUdn(), device.getUdn(), device);
            packet = notifyHeader.toString();
            ssdpListener.sendMulticast(packet.c_str(), packet.length());
            
            Uuid uuid(device.getUdn());
            string nt = device.getDeviceType();
            uuid.setRest(nt);
            
            notifyHeader = makeNotifyAlive(nt, uuid.toString(), device);
            packet = notifyHeader.toString();
            ssdpListener.sendMulticast(packet.c_str(), packet.length());
        }
    }
    void UPnPServer::announceService(const UPnPDevice & device, const UPnPService & service) {
        HttpHeader notifyHeader = makeNotifyAlive(device, service);
        string packet = notifyHeader.toString();
        ssdpListener.sendMulticast(packet.c_str(), packet.length());
    }
    
    void UPnPServer::byebyeDeviceRecursive(const UPnPDevice & device) {
        byebyeDevice(device);
        const vector<UPnPDevice> & embeds = device.getEmbeddedDevices();
        for (size_t i = 0; i < embeds.size(); i++) {
            const UPnPDevice & embed = embeds[i];
            byebyeDevice(embed);
        }
        const vector<UPnPService> & services = device.getServices();
        for (size_t i = 0; i < services.size(); i++) {
            const UPnPService & service = services[i];
            byebyeService(device, service);
        }
    }
    void UPnPServer::byebyeDevice(const UPnPDevice & device) {
        HttpHeader notifyHeader = makeNotifyByebye(device);
        string packet = notifyHeader.toString();
        ssdpListener.sendMulticast(packet.c_str(), packet.length());
        
        if (device.isRootDevice()) {
            notifyHeader = makeNotifyByebye(device.getUdn(), device.getUdn(), device);
            packet = notifyHeader.toString();
            ssdpListener.sendMulticast(packet.c_str(), packet.length());
            
            Uuid uuid(device.getUdn());
            string nt = device.getDeviceType();
            uuid.setRest(nt);
            
            notifyHeader = makeNotifyByebye(nt, uuid.toString(), device);
            packet = notifyHeader.toString();
            ssdpListener.sendMulticast(packet.c_str(), packet.length());
        }
    }
    void UPnPServer::byebyeService(const UPnPDevice & device, const UPnPService & service) {
        HttpHeader notifyHeader = makeNotifyByebye(device, service);
        string packet = notifyHeader.toString();
        ssdpListener.sendMulticast(packet.c_str(), packet.length());
    }
    
    void UPnPServer::onMsearch(const HttpHeader & header, const InetAddress & remoteAddr) {
        
        string st = header.getHeaderFieldIgnoreCase("ST");
        if (Text::equalsIgnoreCase(st, "upnp:rootdevice")) {
            
            vector<UPnPDevice> roots = devices.getRootDevices();
            
            for (size_t i = 0; i < roots.size(); i++) {
                UPnPDevice device = roots[i];
                DatagramSocket socket(remoteAddr.getAddress().c_str(), remoteAddr.getPort());
                HttpHeader responseHeader = makeMsearchResponse(device);
                string packet = responseHeader.toString();
				// logger.logd(packet);
                socket.send(remoteAddr.getAddress().c_str(), remoteAddr.getPort(), packet.c_str(), packet.length());
            }
        }
    }
    
    HttpHeader UPnPServer::makeMsearchResponse(const UPnPDevice & device) {
        
        Uuid uuid(device.getUdn());
        string st = device.getDeviceType();
        if (device.isRootDevice()) {
            st = "upnp:rootdevice";
        }
        uuid.setRest(st);
        
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
    
    HttpHeader UPnPServer::makeNotifyAlive(const UPnPDevice & device) {
        
        Uuid uuid(device.getUdn());
        string nt = device.getDeviceType();
        if (device.isRootDevice()) {
            nt = "upnp:rootdevice";
        }
        uuid.setRest(nt);
        
        return makeNotifyAlive(nt, uuid.toString(), device);
    }
    HttpHeader UPnPServer::makeNotifyAlive(const UPnPDevice & device, const UPnPService & service) {
        
        Uuid uuid(device.getUdn());
        string nt = service.getServiceType();
        uuid.setRest(nt);
        
        string deviceDescriptionLocation = makeDeviceDescriptionUrl(device);
        
        return makeNotifyAlive(nt, uuid.toString(), device);
    }
    HttpHeader UPnPServer::makeNotifyAlive(const string & nt, const string usn, const UPnPDevice & device) {
        string deviceDescriptionLocation = makeDeviceDescriptionUrl(device);
        
        HttpHeader responseHeader;
        responseHeader.setContentLength(0);
        responseHeader.setParts("NOTIFY", "*", "HTTP/1.1");
        responseHeader.setHeaderField("HOST", "239.255.255.250:1900");
        responseHeader.setHeaderField("CACHE-CONTROL", "max-age=1800");
        responseHeader.setHeaderField("NT", nt);
        responseHeader.setHeaderField("USN", usn);
        responseHeader.setHeaderField("NTS", "ssdp:alive");
        responseHeader.setHeaderField("SERVER", "Platform/0.1 UPnP/0.1 App/0.1");
        responseHeader.setHeaderField("LOCATION", deviceDescriptionLocation);
        return responseHeader;
    }
    HttpHeader UPnPServer::makeNotifyByebye(const UPnPDevice & device) {
        
        Uuid uuid(device.getUdn());
        string nt = device.getDeviceType();
        if (device.isRootDevice()) {
            nt = "upnp:rootdevice";
        }
        uuid.setRest(nt);
        
        return makeNotifyByebye(nt, uuid.toString(), device);
    }
    HttpHeader UPnPServer::makeNotifyByebye(const UPnPDevice & device, const UPnPService & service) {
        Uuid uuid(device.getUdn());
        string nt = service.getServiceType();
        uuid.setRest(nt);
        
        return makeNotifyByebye(nt, uuid.toString(), device);
    }
    HttpHeader UPnPServer::makeNotifyByebye(const std::string & nt, const std::string usn, const UPnPDevice & device) {
        
        HttpHeader responseHeader;
        responseHeader.setContentLength(0);
        responseHeader.setParts("NOTIFY", "*", "HTTP/1.1");
        responseHeader.setHeaderField("HOST", "239.255.255.250:1900");
        responseHeader.setHeaderField("NT", nt);
        responseHeader.setHeaderField("USN", usn);
        responseHeader.setHeaderField("NTS", "ssdp:byebye");
        responseHeader.setHeaderField("SERVER", "Platform/0.1 UPnP/0.1 App/0.1");
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
        
        ChunkedBuffer & buffer = request.getChunkedBuffer();
		request.readChunkedBuffer(buffer);
		
		if (request.completeContentRead()) {
            
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

	void UPnPServer::onHttpRequestContent(HttpRequest & request, Packet & packet) {

		DataTransfer * transfer = request.getTransfer();

		if (transfer) {
			if (transfer->isCompleted()) {
			}
		}
	}

	void UPnPServer::onDeviceDescriptionRequest(HttpRequest & request, HttpResponse & response) {

        string udn = getUdnFromHttpRequest(request);
                
        if (devices.hasDevice(udn)) {
            string xml = getDeviceDescription(udn);

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

		response.setStatusCode(200);
		response.setContentType("text/xml");
        response.setContentLength((int)xml.length());
        response.write(xml);
	}
	void UPnPServer::onControlRequest(HttpRequest & request, HttpResponse & response, const UPnPService & service) {
		
		string content;
        ChunkedBuffer & buffer = request.getChunkedBuffer();
		if (buffer.getChunkSize() > 0) {
			content = string(buffer.getChunkData(), buffer.getChunkSize());
		}

		if (content.empty()) {
			response.setStatusCode(404, "wrong request");
			return;
		}

		string actionName = getActionNameFromHttpRequest(request);

		XmlDomParser parser;
		XmlDocument doc = parser.parse(content);

		SoapReader reader;
		XmlNode actionNode = reader.getActionNode(doc.getRootNode());
		UPnPActionParameters params = reader.getActionParametersFromActionNode(actionNode);
        
        UPnPActionRequest actionRequest(service, actionName);
		actionRequest.setParameters(params);
        
        UPnPActionResponse actionResponse;
		
        actionResponse.setResult(UPnPActionResult(false, 500, "Not supported"));
		if (actionRequestHandler) {
			actionRequestHandler->onActionRequest(actionRequest, actionResponse);
		}
        
        if (actionResponse.getResult().isSuccess()) {
            SoapResponseWriter writer;
            writer.setSoapAction(service.getServiceType(), actionName);
            vector<string> names = actionResponse.getParameterNames();
            for (size_t i = 0; i < names.size(); i++) {
                string & name = names[i];
                string value = actionResponse[name];
                writer.setArgument(name, value);
            }
            writer.setPrologue("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
            writer.setShowPrologue(true);
            response.setContentType("");
            response.write(writer.toString());
        }
        
        response.setStatusCode(actionResponse.getResult().getErrorCode(), actionResponse.getResult().getErrorMessage());
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

		string soapAction = request.getHeaderFieldIgnoreCase("SOAPACTION");
		size_t sep = soapAction.find_last_of("#");
		if (sep != string::npos) {
			size_t f = sep + 1;
			size_t end = soapAction.find("\"", f);
			if (end != string::npos) {
				return soapAction.substr(f, end - f);
			} else {
				return soapAction.substr(f);
			}
		}

		return "";
	}

    string UPnPServer::getDeviceDescription(const string & udn) {
        UPnPDeviceXmlWriter writer;
        UPnPDevice & device = devices.getDevice(udn);
        XmlDocument doc = writer.makeDeviceDescriptionXmlDocument(device);
        doc.setPrologue("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        XmlPrinter printer;
		printer.setShowPrologue(true);
        return printer.printDocument(doc);
    }
    
    string UPnPServer::getScpd(const Scpd & scpd) {
		ScpdXmlWriter writer;
		XmlDocument doc = writer.makeScpdXmlDocument(scpd);
		doc.setPrologue("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        XmlPrinter printer;
		printer.setShowPrologue(true);
        return printer.printDocument(doc);
    }
    
    void UPnPServer::setActionRequestHandler(UPnPActionRequestHandler * actionRequestHandler) {
        this->actionRequestHandler = actionRequestHandler;
    }
	
	UrlSerializer & UPnPServer::getUrlSerializer() {
		return urlSerializer;
	}
}
