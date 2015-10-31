#include <iostream>
#include <string>
#include <map>
#include <liboslayer/os.hpp>
#include <liboslayer/Text.hpp>
#include <libhttp-server/HttpClientThreadPool.hpp>
#include <libupnp-tools/SSDPServer.hpp>
#include <libupnp-tools/UPnPDevice.hpp>
#include <libupnp-tools/UPnPService.hpp>
#include <libupnp-tools/UPnPDeviceMaker.hpp>
#include <libupnp-tools/UPnPServiceMaker.hpp>
#include <libupnp-tools/XmlDocument.hpp>
#include <libupnp-tools/XmlDomParser.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace HTTP;
using namespace SSDP;
using namespace UPNP;
using namespace XML;

#define logd printf

static size_t readline(char * buffer, size_t max) {
	fgets(buffer, (int)max - 1, stdin);
	buffer[strlen(buffer) - 1] = 0;
	return strlen(buffer);
}

static void printNotify(HttpHeader & header) {
	string nts = header["NTS"];
	if (Text::equalsIgnoreCase(nts, "ssdp:alive")) {
		//logd("NOTIFY - %s (url: %s)\n", nts.c_str(), header["Location"].c_str());
	} else if (Text::equalsIgnoreCase(nts, "ssdp:byebye")) {
		//logd("NOTIFY - %s\n", nts.c_str());
	} else {
		//logd("NOTIFY - unknown (%s)\n", nts.c_str());
	}
}

static void printMsearch(HttpHeader & header) {
	//logd("M-SEARCH - ST: %s\n", header["ST"].c_str());
}

static void printHttpResponse(HttpHeader & header) {
	//logd("HTTP - %s (url: %s)\n", header["ST"].c_str(), header["Location"].c_str());
}

class UPnPDeviceWriter : public UPnPDevice {
public:
    UPnPDeviceWriter();
    virtual ~UPnPDeviceWriter();
    
    void addEmbeddedDevice();
    void addService();
    void bindScpd();
};

class ServicePosition {
private:
    string udn;
    vector<size_t> deviceIndices;
    size_t serviceIndex;
    string serviceType;
    int traverseDepth;
public:
    ServicePosition();
    virtual ~ServicePosition();
 
    void setUdn(string udn);
    string & getUdn();
    void enterDevice(size_t index);
    void resetTraverse();
    size_t traverseDevice();
    bool hasNextDevice();
    size_t getServiceIndex();
};

ServicePosition::ServicePosition() : traverseDepth(0), serviceIndex(0) {
}

ServicePosition::~ServicePosition() {
}

void ServicePosition::enterDevice(size_t index) {
    deviceIndices.push_back(index);
}

void ServicePosition::resetTraverse() {
    traverseDepth = 0;
}

size_t ServicePosition::traverseDevice() {
    if (!hasNextDevice()) {
        throw Exception("no more index", -1, 0);
    }
    return deviceIndices[traverseDepth++];
}

bool ServicePosition::hasNextDevice() {
    return traverseDepth < deviceIndices.size();
}

size_t ServicePosition::getServiceIndex() {
    return serviceIndex;
}


class DevicePool {
private:
    Semaphore deviceTableLock;
	map<string, UPnPDevice> deviceTable;
public:
    DevicePool() : deviceTableLock(1) {
	}
	virtual ~DevicePool() {
	}
    void clear() {
        deviceTableLock.wait();
        deviceTable.clear();
        deviceTableLock.post();
    }
	UPnPDevice & getDevice(string udn) {
        return deviceTable[udn];
	}
    bool hasDevice(string udn) {
        return deviceTable.find(udn) != deviceTable.end();
    }
	void addDevice(UPnPDevice & device) {
        deviceTableLock.wait();
		deviceTable[device.getUdn()] = device;
        deviceTableLock.post();
	}
	void removeDevice(string udn) {
        deviceTableLock.wait();
		deviceTable.erase(udn);
        deviceTableLock.post();
	}
    UPnPService * traverseService(UPnPDevice & device, ServicePosition & servicePosition) {
        servicePosition.resetTraverse();
        UPnPDevice * currentDevice = &device;
        while (servicePosition.hasNextDevice()) {
            if (!currentDevice) {
                return NULL;
            }
            size_t index = servicePosition.traverseDevice();
            if (index >= currentDevice->getEmbeddedDevices().size()) {
                return NULL;
            }
            currentDevice = &(currentDevice->getEmbeddedDevice(index));
        }
        if (servicePosition.getServiceIndex() >= currentDevice->getServices().size()) {
            return NULL;
        }
        return &(currentDevice->getService(servicePosition.getServiceIndex()));
    }
    void bindScpd(ServicePosition & servicePosition, Scpd & scpd) {
        deviceTableLock.wait();
        string udn = servicePosition.getUdn();
        if (hasDevice(udn)) {
            UPnPService * service = traverseService(getDevice(udn), servicePosition);
            if (service) {
                service->setScpd(scpd);
            }
        }
        deviceTableLock.post();
    }
};

class DeviceDetection {
private:
public:
    DeviceDetection() {
    }
    virtual ~DeviceDetection() {
    }
    
    virtual void onDeviceHelloWithUrl(string url) = 0;
    virtual void onDeviceDescriptionInXml(string xmlDoc) = 0;
    virtual void onScpdInXml(ServicePosition & servicePosition, string xmlDoc) = 0;
    virtual void onDeviceByeBye(string udn) = 0;
};

class SSDPMessageHandler : public OnNotifyHandler, public OnMsearchHandler, public OnHttpResponseHandler {
private:
    DeviceDetection * deviceDetection;
public:
    SSDPMessageHandler() : deviceDetection(NULL) {
    }
    
    virtual ~SSDPMessageHandler() {
    }
    
    virtual void onNotify(HttpHeader & header) {
        printNotify(header);
        if (deviceDetection) {
            string nts = header["NTS"];
            if (Text::equalsIgnoreCase(nts, "ssdp:alive")) {
                deviceDetection->onDeviceHelloWithUrl(header["Location"]);
            } else if (Text::equalsIgnoreCase(nts, "ssdp:byebye")) {
                deviceDetection->onDeviceByeBye(header["USN"]);
            }
            
        }
    }
    virtual void onMsearch(HttpHeader & header) {
        printMsearch(header);
    }
    virtual void onHttpResponse(HttpHeader & header) {
        printHttpResponse(header);
        if (deviceDetection) {
            deviceDetection->onDeviceHelloWithUrl(header["Location"]);
        }
    }
    
    void setDeviceDetection(DeviceDetection * deviceDetection) {
        this->deviceDetection = deviceDetection;
    }
};

class RequestType {
public:
    static const int UNKNOWN;
    static const int DEVICE_DESCRIPTION;
    static const int SCPD;
    
    static const RequestType UNKNOWN_TYPE;
    static const RequestType DEVICE_DESCRIPTION_TYPE;
    static const RequestType SCPD_TYPE;
    
private:
    int type;
    
public:
    RequestType();
    RequestType(int type);
    virtual ~RequestType();
    
    bool operator==(const int & type);
    bool operator==(const RequestType & other);
};

const int RequestType::UNKNOWN = 0;
const int RequestType::DEVICE_DESCRIPTION = 1;
const int RequestType::SCPD = 2;

const RequestType RequestType::UNKNOWN_TYPE(RequestType::UNKNOWN);
const RequestType RequestType::DEVICE_DESCRIPTION_TYPE(RequestType::DEVICE_DESCRIPTION);
const RequestType RequestType::SCPD_TYPE(RequestType::SCPD);

RequestType::RequestType() : type(UNKNOWN) {
}

RequestType::RequestType(int type) : type(type) {
}

RequestType::~RequestType() {
}

bool RequestType::operator==(const int & type) {
    return this->type == type;
}

bool RequestType::operator==(const RequestType & other) {
    return this->type == other.type;
}

class RequestSession {
private:
    DeviceDetection * deviceDetection;
    RequestType type;
    ServicePosition servicePosition;
    
public:
    RequestSession();
    RequestSession(const RequestType & type);
    virtual ~RequestSession();
    void setServicePosition(ServicePosition & servicePosition);
    RequestType getRequestType();
    void setDeviceDetection(DeviceDetection * deviceDetection);
};

RequestSession::RequestSession() : deviceDetection(NULL) {
}

RequestSession::RequestSession(const RequestType & type) : deviceDetection(NULL), type(type) {
}

RequestSession::~RequestSession() {
}

void RequestSession::setServicePosition(ServicePosition & servicePosition) {
    this->servicePosition = servicePosition;
}

RequestType RequestSession::getRequestType() {
    return type;
}

void RequestSession::setDeviceDetection(DeviceDetection * deviceDetection) {
    this->deviceDetection = deviceDetection;
}

class ControlPoint : public DeviceDetection, public HttpResponseHandler<RequestSession> {
private:
    DevicePool constructingDevicePool;
    DevicePool devicePool;
    SSDPServer ssdp;
    SSDPMessageHandler ssdpHandler;
    HttpClientThreadPool<RequestSession> httpClientThreadPool;
    
public:
    ControlPoint();
    virtual ~ControlPoint();
    
    void start();
    void startAsync();
    void poll(unsigned long timeout);
    void stop();
    
    void sendMsearch(string searchType);
    
    virtual void onDeviceHelloWithUrl(string url);
    virtual void onDeviceDescriptionInXml(string xmlDoc);
    virtual void onScpdInXml(ServicePosition & servicePosition, string xmlDoc);
    virtual void onDeviceByeBye(string udn);
    
    UPnPDevice makeDeviceFromXml(XmlNode & xmlNode);
    UPnPService makeServiceFromXml(XmlNode & xmlNode);
    Scpd makeScpdFromXml(XmlNode & xmlNode);
    
    virtual void onResponse(HttpClient<RequestSession> & httpClient, HttpHeader & responseHeader, OS::Socket & socket, RequestSession userData);
};

ControlPoint::ControlPoint() : httpClientThreadPool(10) {
    ssdpHandler.setDeviceDetection(this);
    ssdp.addHttpResponseHandler(&ssdpHandler);
    ssdp.addNotifyHandler(&ssdpHandler);
    ssdp.addMsearchHandler(&ssdpHandler);
    
    httpClientThreadPool.setFollowRedirect(true);
    httpClientThreadPool.setHttpResponseHandler(this);
}

ControlPoint::~ControlPoint() {
}

void ControlPoint::start() {
    ssdp.start();
    httpClientThreadPool.start();
}

void ControlPoint::startAsync() {
    ssdp.startAsync();
    httpClientThreadPool.start();
}

void ControlPoint::poll(unsigned long timeout) {
    ssdp.poll(timeout);
}

void ControlPoint::stop() {
    ssdp.stop();
    httpClientThreadPool.stop();
}

void ControlPoint::sendMsearch(string searchType) {
    ssdp.sendMsearch(searchType);
}

void ControlPoint::onDeviceHelloWithUrl(string url) {
    logd("hello: %s\n", url.c_str());
     httpClientThreadPool.request(Url(url), "GET", NULL, 0, RequestSession(RequestType::DEVICE_DESCRIPTION_TYPE));
}

void ControlPoint::onDeviceDescriptionInXml(string xmlDoc) {
    // TODO: add to constructing device pool
    logd("xmlDoc: %s\n", xmlDoc.c_str());
    XmlDomParser parser;
    UPnPDevice device = UPnPDeviceMaker::makeDeviceWithDeviceDescription(parser.parse(xmlDoc));
    //logd("device description: %s\n", device.getUdn().c_str());
}

void ControlPoint::onScpdInXml(ServicePosition & servicePosition, string xmlDoc) {
    // TODO: bind scpd to device
}

void ControlPoint::onDeviceByeBye(string udn) {
    // TODO: remove device
}

UPnPDevice ControlPoint::makeDeviceFromXml(XmlNode & xmlNode) {
    return UPnPDeviceMaker::makeDeviceWithDeviceNode(xmlNode);
}
UPnPService ControlPoint::makeServiceFromXml(XmlNode & xmlNode) {
    return UPnPServiceMaker::makeServiceWithXmlNode(xmlNode);
}
Scpd ControlPoint::makeScpdFromXml(XmlNode & xmlNode) {
    return UPnPServiceMaker::makeScpdFromXmlNode(xmlNode);
}

void ControlPoint::onResponse(HttpClient<RequestSession> & httpClient, HttpHeader & responseHeader, OS::Socket & socket, RequestSession session) {
    
    if (session.getRequestType() == RequestType::DEVICE_DESCRIPTION) {
        // logd("device descrition received\n");
        
        string dump = HttpResponseDump::dump(responseHeader, socket);
        this->onDeviceDescriptionInXml(dump);
    }
}


int main(int argc, char * args[]) {
    
    ControlPoint cp;

    cp.startAsync();

	while (1) {
		char buffer[1024] = {0,};
		readline(buffer, sizeof(buffer));
		if (!strcmp(buffer, "q")) {
			break;
		}
		if (!strcmp(buffer, "m")) {
			cp.sendMsearch("upnp:rootdevice");
		}
	}

    cp.stop();

	logd("exit\n");

	return 0;
}