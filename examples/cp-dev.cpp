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
#include <libupnp-tools/macros.hpp>
#include <libupnp-tools/Uuid.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace HTTP;
using namespace SSDP;
using namespace UPNP;
using namespace XML;

#define LOG_COMMON 1
#define LOG_CP 0
#define LOG_SSDP 0
#define LOG_APP 1

#define logd(ENABLE, ...) if (ENABLE) printf(__VA_ARGS__)

static size_t readline(char * buffer, size_t max) {
	fgets(buffer, (int)max - 1, stdin);
	buffer[strlen(buffer) - 1] = 0;
	return strlen(buffer);
}

static void printNotify(HttpHeader & header) {
	string nts = header["NTS"];
	if (Text::equalsIgnoreCase(nts, "ssdp:alive")) {
		logd(LOG_SSDP, "NOTIFY - %s (url: %s)\n", nts.c_str(), header["Location"].c_str());
//        logd(LOG_SSDP, "NOTIFY - %s\n", header.toString().c_str());
	} else if (Text::equalsIgnoreCase(nts, "ssdp:byebye")) {
		logd(LOG_SSDP, "NOTIFY - %s\n", nts.c_str());
	} else {
		logd(LOG_SSDP, "NOTIFY - unknown (%s)\n", nts.c_str());
	}
}

static void printMsearch(HttpHeader & header) {
	logd(LOG_SSDP, "M-SEARCH - ST: %s\n", header["ST"].c_str());
}

static void printHttpResponse(HttpHeader & header) {
	logd(LOG_SSDP, "HTTP - %s (url: %s)\n", header["ST"].c_str(), header["Location"].c_str());
//    logd(LOG_SSDP, "HTTP - %s\n", header.toString().c_str());
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
    mutable string udn;
    mutable vector<size_t> deviceIndices;
    mutable size_t serviceIndex;
    mutable string serviceType;
    mutable size_t traverseDepth;
    mutable string scpdurl;
    
public:
    ServicePosition();
    virtual ~ServicePosition();
 
    void setUdn(const string & udn) const;
    string & getUdn() const;
    void enterDevice(size_t index) const;
    void resetTraverse() const;
    size_t traverseDevice() const;
    bool hasNextDevice() const;
    size_t getServiceIndex() const;
    void setDeviceIndices(vector<size_t> & indices);
    void setSerivceIndex(size_t index);
    void setScpdUrl(const string & scpdurl);
    string getScpdUrl();
    string toString() const;
    void setServiceType(string serviceType);
};

ServicePosition::ServicePosition() : traverseDepth(0), serviceIndex(0) {
}

ServicePosition::~ServicePosition() {
}

void ServicePosition::setUdn(const string & udn) const {
	this->udn = udn;
}
string & ServicePosition::getUdn() const {
	return udn;
}

void ServicePosition::enterDevice(size_t index) const {
    deviceIndices.push_back(index);
}

void ServicePosition::resetTraverse() const {
    traverseDepth = 0;
}

size_t ServicePosition::traverseDevice() const {
    if (!hasNextDevice()) {
        throw Exception("no more index", -1, 0);
    }
    return deviceIndices[traverseDepth++];
}

bool ServicePosition::hasNextDevice() const {
    return traverseDepth < deviceIndices.size();
}

size_t ServicePosition::getServiceIndex() const {
    return serviceIndex;
}

void ServicePosition::setDeviceIndices(vector<size_t> & indices) {
    this->deviceIndices = indices;
}

void ServicePosition::setSerivceIndex(size_t index) {
    this->serviceIndex = index;
}

void ServicePosition::setScpdUrl(const string & scpdurl) {
    this->scpdurl = scpdurl;
}

string ServicePosition::getScpdUrl() {
    return scpdurl;
}

string ServicePosition::toString() const {
    string str;
    LOOP_VEC(deviceIndices, i) {
        size_t x = deviceIndices[i];
        str.append(Text::toString((int)x));
        str.append(" ");
    }
    str.append("[" + Text::toString((int)serviceIndex) + "]");
    str.append(" - " + serviceType);
    return str;
}

void ServicePosition::setServiceType(string serviceType) {
    this->serviceType = serviceType;
}

class ServicePositionMaker {
private:
    string udn;
    vector<size_t> deviceIndices;
    size_t currentLevel;
public:
    ServicePositionMaker(string udn);
    virtual ~ServicePositionMaker();
    ServicePosition makeServicePosition(size_t index, UPnPService & service);
    void enter();
    void setDeviceIndex(size_t index);
    void leave();
};

ServicePositionMaker::ServicePositionMaker(string udn) : udn(udn), currentLevel(0) {
}

ServicePositionMaker::~ServicePositionMaker() {
}

ServicePosition ServicePositionMaker::makeServicePosition(size_t index, UPnPService & service) {
    ServicePosition sp;
    sp.setUdn(udn);
    sp.setScpdUrl(service["SCPDURL"]);
    sp.setServiceType(service["serviceType"]);
    if (currentLevel > 0) {
        vector<size_t> indices(deviceIndices.begin(), deviceIndices.begin() + currentLevel);
        sp.setDeviceIndices(indices);
    }
    sp.setSerivceIndex(index);
    return sp;
}

void ServicePositionMaker::enter() {
    currentLevel++;
    if (currentLevel >= deviceIndices.size()) {
        deviceIndices.push_back(0);
    }
}

void ServicePositionMaker::setDeviceIndex(size_t index) {
    deviceIndices[currentLevel - 1] = index;
}

void ServicePositionMaker::leave() {
    currentLevel--;
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
    void updateDevice(UPnPDevice & device) {
        deviceTableLock.wait();
        deviceTable[device.getUdn()] = device;
        deviceTableLock.post();
    }
    
	void removeDevice(string udn) {
        deviceTableLock.wait();
		deviceTable.erase(udn);
        deviceTableLock.post();
	}
    
    UPnPService * traverseService(const UPnPDevice & device, const ServicePosition & servicePosition) {
        servicePosition.resetTraverse();
        const UPnPDevice * currentDevice = &device;
        while (servicePosition.hasNextDevice()) {
            if (!currentDevice) {
                throw -1;
//                return NULL;
            }
            size_t index = servicePosition.traverseDevice();
            if (index >= currentDevice->getEmbeddedDevices().size()) {
                throw -1;
//                return NULL;
            }
            currentDevice = &(currentDevice->getEmbeddedDevice(index));
        }
        if (servicePosition.getServiceIndex() >= currentDevice->getServices().size()) {
            throw -1;
//            return NULL;
        }
        return &(currentDevice->getService(servicePosition.getServiceIndex()));
    }
    
    void bindScpd(const ServicePosition & servicePosition, const Scpd & scpd) {
        deviceTableLock.wait();
        string udn = servicePosition.getUdn();
        if (hasDevice(udn)) {
            UPnPService * service = traverseService(getDevice(udn), servicePosition);
            if (service) {
                logd(LOG_CP, "bind service: %s\n", service->getServiceType().c_str());
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
    
    virtual void onDeviceHelloWithUrl(const string & url, const HttpHeader & header) = 0;
    virtual void onDeviceDescriptionInXml(string baseUrl, string xmlDoc) = 0;
    virtual void onScpdInXml(const ServicePosition & servicePosition, string xmlDoc) = 0;
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
                deviceDetection->onDeviceHelloWithUrl(header["Location"], header);
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
            deviceDetection->onDeviceHelloWithUrl(header["Location"], header);
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
    void setServicePosition(const ServicePosition & servicePosition);
    ServicePosition & getServicePosition();
    RequestType getRequestType();
    void setDeviceDetection(DeviceDetection * deviceDetection);
};

RequestSession::RequestSession() : deviceDetection(NULL) {
}

RequestSession::RequestSession(const RequestType & type) : deviceDetection(NULL), type(type) {
}

RequestSession::~RequestSession() {
}

void RequestSession::setServicePosition(const ServicePosition & servicePosition) {
    this->servicePosition = servicePosition;
}

ServicePosition & RequestSession::getServicePosition() {
    return servicePosition;
}

RequestType RequestSession::getRequestType() {
    return type;
}

void RequestSession::setDeviceDetection(DeviceDetection * deviceDetection) {
    this->deviceDetection = deviceDetection;
}

class Filter {
private:
    vector<string> filterTypes;
public:
    Filter();
    virtual ~Filter();
    
    void addFilterType(const string & type);
    bool filter(const string & type);
};

Filter::Filter() {
}

Filter::~Filter() {
}

void Filter::addFilterType(const string & type) {
    filterTypes.push_back(type);
}

bool Filter::filter(const string & type) {
    LOOP_VEC(filterTypes, i) {
        if (!filterTypes[i].compare(type)) {
            return true;
        }
    }
    return false;
}

class DeviceAddRemoveListener {
private:
public:
    DeviceAddRemoveListener() {}
    virtual ~DeviceAddRemoveListener() {}
    virtual void onDeviceAdded(UPnPDevice & device) = 0;
    virtual void onDeviceRemoved(UPnPDevice & device) = 0;
};

class ControlPoint : public DeviceDetection, public HttpResponseHandler<RequestSession> {
private:
    DevicePool devicePool;
    SSDPServer ssdp;
    SSDPMessageHandler ssdpHandler;
    HttpClientThreadPool<RequestSession> httpClientThreadPool;
    Filter filter;
    DeviceAddRemoveListener * deviceListener;
    
public:
    ControlPoint();
    virtual ~ControlPoint();
    
    void start();
    void startAsync();
    void poll(unsigned long timeout);
    void stop();
    
    void sendMsearch(string searchType);
    
    virtual void onDeviceHelloWithUrl(const string & url, const HttpHeader & header);
    virtual void onDeviceDescriptionInXml(string baseUrl, string xmlDoc);
    virtual void onScpdInXml(const ServicePosition & servicePosition, string xmlDoc);
    virtual void onDeviceByeBye(string udn);
    
    UPnPDevice makeDeviceFromXml(XmlNode & xmlNode);
    UPnPService makeServiceFromXml(XmlNode & xmlNode);
    Scpd makeScpdFromXml(XmlNode & xmlNode);
    
    virtual void onResponse(HttpClient<RequestSession> & httpClient, HttpHeader & responseHeader, OS::Socket & socket, RequestSession userData);
    
    void setFilter(const Filter & filter);
    vector<ServicePosition> makeServicePositions(ServicePositionMaker & maker, const UPnPDevice & device);
    void setDeviceAddRemoveListener(DeviceAddRemoveListener * deviceListener);
};

ControlPoint::ControlPoint() : httpClientThreadPool(10), deviceListener(NULL) {
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
	logd(LOG_CP, "send msearch/%s\n", searchType.c_str());
    ssdp.sendMsearch(searchType);
}

void ControlPoint::onDeviceHelloWithUrl(const string & url, const HttpHeader & header) {
    string usn = header["USN"];
    Uuid uuid(usn);
    if (!filter.filter(uuid.getRest())) {
        return;
    }
    if (devicePool.hasDevice(uuid.getUuid())) {
        return;
    }
    UPnPDevice device;
    device.setUdn(uuid.getUuid());
    devicePool.addDevice(device);
    logd(LOG_CP, "hello / uuid: %s / rest: %s\n", uuid.getUuid().c_str(), uuid.getRest().c_str());
    httpClientThreadPool.request(Url(url), "GET", NULL, 0, RequestSession(RequestType::DEVICE_DESCRIPTION_TYPE));
}

void ControlPoint::onDeviceDescriptionInXml(string baseUrl, string xmlDoc) {
    XmlDomParser parser;
    UPnPDevice device = UPnPDeviceMaker::makeDeviceWithDeviceDescription(parser.parse(xmlDoc));
    logd(LOG_CP, "onDeviceDescriptionInXml() / udn : %s\n", device.getUdn().c_str());
    
    devicePool.updateDevice(device);
    if (deviceListener) {
        deviceListener->onDeviceAdded(device);
    }
    
    ServicePositionMaker maker(device.getUdn());
    vector<ServicePosition> servicePositions = makeServicePositions(maker, device);

	LOOP_VEC(servicePositions, i) {
		ServicePosition & sp = servicePositions[i];
		string scpdurl = sp.getScpdUrl();
		Url url = Url(baseUrl);
		url.setPath(scpdurl);
        RequestSession session(RequestType::SCPD_TYPE);
        session.setServicePosition(sp);
		httpClientThreadPool.request(url, "GET", NULL, 0, session);
        logd(LOG_CP, "scpd position: %s\n", sp.toString().c_str());
	}
}

void ControlPoint::onScpdInXml(const ServicePosition & servicePosition, string xmlDoc) {
    // TODO: bind scpd to device
	XmlDomParser parser;
    Scpd scpd = UPnPServiceMaker::makeScpdWithXmlDocument("", parser.parse(xmlDoc));
    logd(LOG_CP, "scpd position: %s\n", servicePosition.toString().c_str());
    
    devicePool.bindScpd(servicePosition, scpd);
}

void ControlPoint::onDeviceByeBye(string udn) {
    if (!devicePool.hasDevice(udn)) {
        return;
    }
    
    if (deviceListener) {
        deviceListener->onDeviceRemoved(devicePool.getDevice(udn));
    }
    devicePool.removeDevice(udn);
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
        string dump = HttpResponseDump::dump(responseHeader, socket);
		char baseUrl[1024] = {0,};
		snprintf(baseUrl, sizeof(baseUrl), "http://%s:%d", socket.getHost(), socket.getPort());
        this->onDeviceDescriptionInXml(baseUrl, dump);
	} else if (session.getRequestType() == RequestType::SCPD) {
		string dump = HttpResponseDump::dump(responseHeader, socket);
		this->onScpdInXml(session.getServicePosition(), dump);
	}
}

void ControlPoint::setFilter(const Filter & filter) {
    this->filter = filter;
}

vector<ServicePosition> ControlPoint::makeServicePositions(ServicePositionMaker & maker, const UPnPDevice & device) {
    
    vector<ServicePosition> servicePositions;
    vector<UPnPService> & services = device.getServices();
    LOOP_VEC(services, i) {
        servicePositions.push_back(maker.makeServicePosition(i, services[i]));
    }
    
    vector<UPnPDevice> & embeddedDevices = device.getEmbeddedDevices();
    LOOP_VEC(embeddedDevices, i) {
        maker.enter();
        maker.setDeviceIndex(i);
        vector<ServicePosition> append = makeServicePositions(maker, embeddedDevices[i]);
        servicePositions.insert(servicePositions.end(), append.begin(), append.end());
        maker.leave();
    }
    
    return servicePositions;
}

void ControlPoint::setDeviceAddRemoveListener(DeviceAddRemoveListener * deviceListener) {
    this->deviceListener = deviceListener;
}

class MyDeviceListener : public DeviceAddRemoveListener {
private:
public:
    MyDeviceListener() {
        
    }
    virtual ~MyDeviceListener() {
        
    }
    
    virtual void onDeviceAdded(UPnPDevice & device) {
        logd(LOG_APP, "added: %s (%s)\n", device.getUdn().c_str(), device.getFriendlyName().c_str());
    }
    virtual void onDeviceRemoved(UPnPDevice & device) {
        logd(LOG_APP, "removed: %s (%s)\n", device.getUdn().c_str(), device.getFriendlyName().c_str());
    }
};

int main(int argc, char * args[]) {
    
    ControlPoint cp;
    
    MyDeviceListener listener;
    cp.setDeviceAddRemoveListener(&listener);
    
    Filter filter;
    filter.addFilterType("upnp:rootdevice");
    cp.setFilter(filter);

    cp.startAsync();

	logd(LOG_COMMON, "start\n");

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

	logd(LOG_COMMON, "exit\n");

	return 0;
}