#include "UPnPServer.hpp"
#include "Uuid.hpp"
#include <liboslayer/Text.hpp>
#include <liboslayer/Logger.hpp>
#include <libhttp-server/FixedTransfer.hpp>
#include "XmlDomParser.hpp"
#include "XmlDocumentPrinter.hpp"
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
     *
     */
    
    UPnPEventSubscribeInfo::UPnPEventSubscribeInfo() : seq(0) {
    }
    
    UPnPEventSubscribeInfo::UPnPEventSubscribeInfo(const string & sid, UPnPService & service, vector<string> & callbacks) : sid(sid), service(service), callbacks(callbacks), seq(0) {
        
    }
    UPnPEventSubscribeInfo::~UPnPEventSubscribeInfo() {
    }
    
    bool UPnPEventSubscribeInfo::empty() {
        return sid.empty();
    }
    
    string UPnPEventSubscribeInfo::getSid() {
        return sid;
    }
    
    vector<string> & UPnPEventSubscribeInfo::getCallbacks() {
        return callbacks;
    }
    
    UPnPService & UPnPEventSubscribeInfo::getService() {
        return service;
    }
    
    void UPnPEventSubscribeInfo::initSeq() {
        seq = 0;
    }
    unsigned int UPnPEventSubscribeInfo::nextSeq() {
        return seq++;
    }
    unsigned int UPnPEventSubscribeInfo::getSeq() {
        return seq;
    }
    
    
    /**
     *
     */
    
    UPnPEventSubscriberPool::UPnPEventSubscriberPool() {
        
    }
    UPnPEventSubscriberPool::~UPnPEventSubscriberPool() {
        
    }
    
    string UPnPEventSubscriberPool::registerSubscriber(UPnPService & service, vector<string> & callbackUrls) {
        string sid = generateSid();
        subscribers[sid] = UPnPEventSubscribeInfo(sid, service, callbackUrls);
        return sid;
    }
    void UPnPEventSubscriberPool::unregisterSubscriber(const string & sid) {
        subscribers.erase(sid);
    }
    
    UPnPEventSubscribeInfo UPnPEventSubscriberPool::getSubscriberInfo(const string & sid) {
        return subscribers[sid];
    }
    
    UPnPEventSubscribeInfo UPnPEventSubscriberPool::getSubscriberInfo(UPnPService & service) {
        for (map<string, UPnPEventSubscribeInfo>::iterator iter = subscribers.begin(); iter != subscribers.end(); iter++) {
            if (!iter->second.getService().getEventSubscribeUrl().compare(service.getEventSubscribeUrl())) {
                return iter->second;
            }
        }
        
        return UPnPEventSubscribeInfo();
    }
    
    string UPnPEventSubscriberPool::generateSid() {
        return Uuid::generateUuid();
    }
    
    
    /**
     * @brief UPnPServer
     */

	UPnPServer::UPnPServer(int port) : TimerEvent(false), actionRequestHandler(NULL), httpServer(port), pollingThread(NULL), urlSerializer(""), idx(0), httpClientThreadPool(5) {
        
        registerPollee(&timer);
        registerSelectablePollee(&ssdpListener);

		httpServer.registerRequestHandler("/*", this);

        ssdpListener.addMsearchHandler(this);
	}
	
	UPnPServer::~UPnPServer() {
        stop();
	}
    
    void UPnPServer::onTimerTriggered() {
    }
	
	void UPnPServer::start() {
        ssdpListener.start();
        httpServer.start();
        timer.start();
        
        scheduleRepeatableRelativeTimer(0, -1, Timer::SECOND);
        timer.setTimerEvent(this);
        
        httpClientThreadPool.start();
	}
    
    void UPnPServer::startAsync() {
        
        start();
        
        httpServer.startAsync();
        
        if (!pollingThread) {
            pollingThread = new PollingThread(this, 100);
            pollingThread->start();
        }
    }
	
	void UPnPServer::stop() {
        
        httpClientThreadPool.stop();
        
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
    
    void UPnPServer::poll(unsigned long timeout) {
        SelectorPoller::poll(timeout);
        httpClientThreadPool.collectUnflaggedThreads();
    }
    
    UPnPDevice UPnPServer::getDevice(const string & udn) {
        return devicePool.getDevice(udn);
    }
    
    void UPnPServer::registerDeviceWithXml(const string & xmlDoc) {
        XmlDomParser parser;
        XmlDocument dom = parser.parse(xmlDoc);
        UPnPDevice device = UPnPDeviceMaker::makeDeviceFromDeviceDescription("file://./", dom);
        registerDevice(device);
    }
    
    void UPnPServer::registerDevice(const UPnPDevice & device) {
        if (!devicePool.hasDevice(device.getUdn())) {
            devicePool.addDevice(device);
            announceDeviceRecursive(device);
        }
    }
    void UPnPServer::unregisterDevice(const std::string & udn) {
        if (devicePool.hasDevice(udn)) {
            UPnPDevice & device = devicePool.getDevice(udn);
            byebyeDeviceRecursive(device);
            devicePool.removeDevice(udn);
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
            
            vector<UPnPDevice> roots = devicePool.getRootDevices();
            
            for (size_t i = 0; i < roots.size(); i++) {
                UPnPDevice device = roots[i];
                DatagramSocket socket(remoteAddr.getAddress().c_str(), remoteAddr.getPort());
                HttpHeader responseHeader = makeMsearchResponse(device);
                string packet = responseHeader.toString();
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
        InetAddress addr = NetworkUtil::selectDefaultAddress();
        addr.setPort(httpServer.getPort());
        return addr;
    }
    
    void UPnPServer::onHttpRequest(HttpRequest & request, HttpResponse & response) {
        
        response.setStatusCode(404);
        response.getHeader().setConnection("close");
        
        logger.logv(request.getPath());
    }

	void UPnPServer::onHttpRequestContent(HttpRequest & request, HttpResponse & response, Packet & packet) {
    }
    
    void UPnPServer::onHttpRequestContentCompleted(HttpRequest &request, HttpResponse &response) {
        
        string path = request.getPath();
        
        if (urlSerializer.isDeviceDescriptionRequest(path)) {
            onDeviceDescriptionRequest(request, response);
        } else {
            
            string udn = urlSerializer.getUdnFromUrlPath(path);
            if (!udn.empty()) {
                
                udn = "uuid:" + udn;
                if (devicePool.hasDevice(udn)) {
                    UPnPDevice device = devicePool.getDevice(udn);
                    if (device.hasServiceWithPropertyRecursive("SCPDURL", path)) {
                        onScpdRequest(request, response, device.getServiceWithPropertyRecursive("SCPDURL", path));
                    } else if (device.hasServiceWithPropertyRecursive("controlURL", path)) {
                        onControlRequest(request, response, device.getServiceWithPropertyRecursive("controlURL", path));
                    } else if (device.hasServiceWithPropertyRecursive("eventSubURL", path)) {
                        onEventSubRequest(request, response, device.getServiceWithPropertyRecursive("eventSubURL", path));
                    }
                }
            }
        }
	}

	void UPnPServer::onDeviceDescriptionRequest(HttpRequest & request, HttpResponse & response) {

        string udn = getUdnFromHttpRequest(request);
                
        if (devicePool.hasDevice(udn)) {
            string xml = getDeviceDescription(udn);

			response.setStatusCode(200);
			response.setContentType("text/xml");
            setFixedTransfer(response, xml);

        } else {
            response.setStatusCode(404, "No device found");
            setFixedTransfer(response, "No device found...");
        }

	}
	void UPnPServer::onScpdRequest(HttpRequest & request, HttpResponse & response, UPnPService & service) {

		string xml = getScpd(service.getScpd());

		response.setStatusCode(200);
		response.setContentType("text/xml");
        setFixedTransfer(response, xml);
	}
	void UPnPServer::onControlRequest(HttpRequest & request, HttpResponse & response, UPnPService & service) {
		
        AutoRef<DataTransfer> transfer = request.getTransfer();
        
        if (!transfer.empty()) {
            string content = transfer->getString();
            
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
                setFixedTransfer(response, writer.toString());
            }
            
            response.setStatusCode(actionResponse.getResult().getErrorCode(), actionResponse.getResult().getErrorMessage());
            
        }
	}
	void UPnPServer::onEventSubRequest(HttpRequest & request, HttpResponse & response, UPnPService & service) {
        
        string path = request.getPath();
        
        string callback = request.getHeaderField("CALLBACK");
        vector<string> callbackUrls = parseCallbackUrls(callback);
        
        string sid = subsriberPool.registerSubscriber(service, callbackUrls);
        
        response.setStatusCode(200);
        response.getHeader().setHeaderField("SID", "uuid:" + sid);
        response.getHeader().setHeaderField("TIMEOUT", "Second-1800");
        response.setContentLength(0);
        
        if (eventSubscribeListener) {
            eventSubscribeListener->onEventSubsribe(service);
        }
	}
    
    
    vector<string> UPnPServer::parseCallbackUrls(const string & callbackPhrase) {
        
        vector<string> urls;
        
        size_t f = callbackPhrase.find("<");
        while (f != string::npos) {
            size_t e = callbackPhrase.find(">", f);
            if (e != string::npos) {
                size_t s = f + 1;
                string url = callbackPhrase.substr(s, e - s);
                urls.push_back(url);
            } else {
                break;
            }
            
            f = callbackPhrase.find("<", e + 1);
        }
        
        return urls;
    }
    
    void UPnPServer::noitfyPropertyChanged(UPnPService & service, UTIL::LinkedStringMap & values) {

        UPnPEventSubscribeInfo info = subsriberPool.getSubscriberInfo(service);
        vector<string> callbacks = info.getCallbacks();
        
        StringMap fields;
        fields["NT"] = "upnp:event";
        fields["NTS"] = "upnp:propchange";
        fields["SEQ"] = Text::toString(info.nextSeq());
        
        string content = toPropertySetXmlString(values);
        
        for (vector<string>::iterator iter = callbacks.begin(); iter != callbacks.end(); iter++) {
            sendHttpRequest(*iter, "NOTIFY", fields, content, NULL);
        }
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
        UPnPDevice & device = devicePool.getDevice(udn);
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
    
    void UPnPServer::sendHttpRequest(const Url & url, const string & method, const StringMap & additionalFields, const string & content, UserData * userData) {
        httpClientThreadPool.collectUnflaggedThreads();
        httpClientThreadPool.setRequest(url, method, additionalFields, new FixedTransfer(content.c_str(), content.length()), userData);
    }
    void UPnPServer::onRequestComplete(Url & url, HttpResponse & response, const string & content, UserData * userData) {
        
    }
    void UPnPServer::onRequestError(Exception & e, Url & url, UserData * userData) {
        
    }
    
    string UPnPServer::toPropertySetXmlString(LinkedStringMap props) {
        XmlParseCursor cursor;
        XmlNode propertySetNode;
        propertySetNode.setNamespace("e");
        propertySetNode.setTagName("propertyset");
        propertySetNode.setAttribute("xmlns:e", "urn:schemas-upnp-org:event-1-0");
        cursor.enter(propertySetNode);
        
        for (size_t i = 0; i < props.size(); i++) {
            NameValue & nv = props.getByIndex(i);
            string name = nv.getName();
            string value = nv.getValue();
            
            XmlNode propertyNode;
            propertyNode.setNamespace("e");
            propertyNode.setTagName("property");
            cursor.enter(propertyNode);
            
            XmlNode nameNode;
            nameNode.setTagName(name);
            cursor.enter(nameNode);
            
            XmlNode valueNode;
            valueNode.setText(value);
            cursor.enter(valueNode);
            
            cursor.leave();
            cursor.leave();
            cursor.leave();
        }
        
        cursor.leave();
        
        XmlDocument doc;
        doc.setRootNode(cursor.getRootNode());
        doc.setPrologue("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        
        XmlPrinter printer;
        printer.setShowPrologue(true);
        string content = printer.printDocument(doc);
        
        return content;
    }
	
	UrlSerializer & UPnPServer::getUrlSerializer() {
		return urlSerializer;
	}
    
    void UPnPServer::setEventSubscribeListener(UPnPEventSubscribeListener * eventSubscribeListener) {
        this->eventSubscribeListener = eventSubscribeListener;
    }
}
