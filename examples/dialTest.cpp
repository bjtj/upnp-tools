#include <string>
#include <map>
#include <vector>
#include <liboslayer/os.hpp>
#include <liboslayer/DatagramSocket.hpp>
#include <liboslayer/Utils.hpp>
#include <libhttp-server/AnotherHttpClient.hpp>
#include <libhttp-server/FixedTransfer.hpp>
#include <libupnp-tools/SSDPServer.hpp>
#include <libupnp-tools/XmlDomParser.hpp>
#include <libupnp-tools/XmlNodeFinder.hpp>
#include <libupnp-tools/Uuid.hpp>

using namespace std;
using namespace OS;
using namespace XOS;
using namespace UTIL;
using namespace HTTP;
using namespace SSDP;
using namespace XML;
using namespace UPNP;

class SimpleDevice : public UserData {
private:
    LinkedStringMap properties;
public:
    SimpleDevice() {
    }
    virtual ~SimpleDevice() {
    }
    
    string getUdn() {
        return properties["udn"];
    }
    void setUdn(const string & udn) {
        properties["udn"] = udn;
    }
    string getApplicationUrl() {
        return properties["applicationUrl"];
    }
    void setApplicationUrl(const string & applicationUrl) {
        properties["applicationUrl"] = applicationUrl;
    }
    string getFriendlyName() {
        return properties["friendlyName"];
    }
    void setFriendlyName(const string & friendlyName) {
        properties["friendlyName"] = friendlyName;
    }
};

class DeviceManager : public OnNotifyHandler, public OnHttpResponseHandler, public OnResponseListener {
private:
    SSDPServer server;
    map<string, SimpleDevice> devices;
public:
    DeviceManager() {
        server.addNotifyHandler(this);
        server.addHttpResponseHandler(this);
    }
    virtual ~DeviceManager() {
    }
    
    void sendMsearch(const string & type) {
        server.sendMsearch(type);
    }
    
    void start() {
        server.startAsync();
    }
    void stop() {
        server.stop();
    }
    
    vector<SimpleDevice> getDevices() {
        vector<SimpleDevice> ret;
        for (map<string, SimpleDevice>::iterator iter = devices.begin(); iter != devices.end(); iter++) {
            ret.push_back(iter->second);
        }
        return ret;
    }
    
    virtual void onNotify(const HTTP::HttpHeader & header) {
        
        string nts = header.getHeaderFieldIgnoreCase("NTS");
        
        if (Text::equalsIgnoreCase(nts, "ssdp:alive")) {
            
            string usn = header.getHeaderFieldIgnoreCase("USN");
            Uuid uuid(usn);
            
            if (filterDeviceType(usn)) {
                
                string location = header.getHeaderFieldIgnoreCase("LOCATION");
                onDeviceLocationFound(uuid.getUuid(), location);
            }
        }
        
        if (Text::equalsIgnoreCase(nts, "ssdp:byebye")) {
            string usn = header.getHeaderFieldIgnoreCase("USN");
            devices.erase(usn);
        }
    }
    
    virtual void onHttpResponse(const HttpHeader & header) {
        
		string usn = header.getHeaderFieldIgnoreCase("USN");
        Uuid uuid(usn);
        
        if (filterDeviceType(usn)) {
            
            string location = header.getHeaderFieldIgnoreCase("LOCATION");
            onDeviceLocationFound(uuid.getUuid(), location);
        }
    }
    
    bool filterDeviceType(const string & usn) {
        
        if (usn.empty()) {
            return false;
        }
        
        if (Text::endsWith(usn, "urn:schemas-upnp-org:device:tvdevice:1")) {
            return true;
        }
        
        if (Text::endsWith(usn, "urn:dial-multiscreen-org:service:dial:1")) {
            return true;
        }

		/*if (Text::endsWith(usn, "upnp:rootdevice")) {
            return true;
        }*/
        
        return false;
    }
    
    void onDeviceLocationFound(const string & udn, const string & location) {
        Url url(location);
        AnotherHttpClient client(url);
        client.setOnResponseListener(this);
        client.setRequest("GET", LinkedStringMap(), NULL);
        SimpleDevice * device = new SimpleDevice;
        device->setUdn(udn);
        client.setUserData(device);
        client.execute();
    }
    
    virtual void onResponseHeader(HttpResponse & response, AutoRef<UserData> userData) {
        if (response.getHeader().isChunkedTransfer()) {
            response.setTransfer(AutoRef<DataTransfer>(new ChunkedTransfer));
        } else if (response.getHeader().getContentLength() > 0) {
            response.setTransfer(AutoRef<DataTransfer>(new FixedTransfer(response.getHeader().getContentLength())));
        } else {
            // do nothing
        }
    }
    virtual void onTransferDone(HttpResponse & response, DataTransfer * transfer, AutoRef<UserData> userData) {
        
        if (transfer) {
            string content = transfer->getString();
            XmlDomParser parser;
            XmlDocument doc = parser.parse(content);
            string friendlyName = XmlNodeFinder::getContentByTagName(doc.getRootNode(), "friendlyName");
            
            string applicationUrl = response.getHeader().getHeaderField("Application-URL");
            
            string udn = ((SimpleDevice&)*userData).getUdn();
            
            SimpleDevice device;
            device.setUdn(udn);
            device.setFriendlyName(friendlyName);
            device.setApplicationUrl(applicationUrl);
            
            devices[udn] = device;
        }
    }
    virtual void onError(OS::Exception & e, AutoRef<UserData> userData) {
    }
};

class FakeDialClient : public OnResponseListener {
private:
    string appName;
    string applicationUrl;
    string dataUrl;
public:
    FakeDialClient() {
    }
    virtual ~FakeDialClient() {
    }
    
    void setAppName(const string & appName) {
        this->appName = appName;
    }
    void setApplicationUrl(const string & applicationUrl) {
        this->applicationUrl = applicationUrl;
    }
    void setDataUrl(const string & dataUrl) {
        this->dataUrl = dataUrl;
    }
    
    void requestStatus() {
        Url url(applicationUrl + appName);
        AnotherHttpClient client(url);
        client.setOnResponseListener(this);
        client.setRequest("GET", LinkedStringMap(), NULL);
        client.execute();
    }
    void requestLaunch(const string & data) {
        Url url(applicationUrl + appName);
        AnotherHttpClient client(url);
        client.setOnResponseListener(this);
        client.setRequest("POST", LinkedStringMap(), new FixedTransfer(data));
        client.execute();
    }
    void requestStop() {
        Url url(applicationUrl + appName);
        AnotherHttpClient client(url);
        client.setOnResponseListener(this);
        client.setRequest("DELETE", LinkedStringMap(), NULL);
        client.execute();
    }
    void requestDialData(const string & data) {
        Url url(dataUrl);
        AnotherHttpClient client(url);
        client.setOnResponseListener(this);
        client.setRequest("POST", LinkedStringMap(), new FixedTransfer(data));
        client.execute();
    }
    
    virtual void onResponseHeader(HttpResponse & response, AutoRef<UserData> userData) {
        if (response.getHeader().isChunkedTransfer()) {
            response.setTransfer(AutoRef<DataTransfer>(new ChunkedTransfer));
        } else if (response.getHeader().getContentLength() > 0) {
            response.setTransfer(AutoRef<DataTransfer>(new FixedTransfer(response.getHeader().getContentLength())));
        } else {
            // do nothing
        }
    }
    virtual void onTransferDone(HttpResponse & response, DataTransfer * transfer, AutoRef<UserData> userData) {
        printf("RESP HEADER:\n");
        printf("%s", response.getHeader().toString().c_str());
        printf("RESP CONTENT:\n");
        printf("%s\n", transfer ? transfer->getString().c_str() : "<no data>");
    }
    virtual void onError(OS::Exception & e, AutoRef<UserData> userData) {
        printf("Error / %s\n", e.getMessage().c_str());
    }
};

size_t readline(char * buffer, size_t max) {
    if (fgets(buffer, (int)max - 1, stdin)) {
        buffer[strlen(buffer) - 1] = 0;
        return strlen(buffer);
    }
    return 0;
}

void s_print_device_list(vector<SimpleDevice> & devices) {
    
    printf("Device list (%ld)\n", devices.size());
    
    for (size_t i = 0; i < devices.size(); i++) {
        SimpleDevice & device = devices[i];
        
        string udn = device.getUdn();
        string friendlyName = device.getFriendlyName();
        
        printf("[%ld] UDN : %s\n", i, udn.c_str());
        printf(" -- Friendly Name : %s\n", friendlyName.c_str());
    }
}

bool isDigitString(const string & str) {
    
    if (str.empty()) {
        return false;
    }
    
    return str.find_first_not_of("0123456789") == string::npos;
}

int main(int argc, char * args[]) {
    
    DeviceManager dm;
    FakeDialClient client;
    
    dm.start();
    
    while (1) {
        char buffer[1024] = {0,};
        readline(buffer, sizeof(buffer));
        
        if (!strcmp(buffer, "q")) {
            break;
        }
        
        if (buffer[0] == '\0') {
            vector<SimpleDevice> devices = dm.getDevices();
            s_print_device_list(devices);
            continue;
        }
        
        if (isDigitString(buffer)) {
            int index = Text::toInt(buffer);
            vector<SimpleDevice> devices = dm.getDevices();
            if (index >= 0 && (size_t)index < devices.size()) {
                SimpleDevice & device = devices[index];
                string applicationUrl = device.getApplicationUrl();
                client.setApplicationUrl(applicationUrl);
                
                printf("Set ApplicationURL: %s\n", applicationUrl.c_str());
                
            }
            continue;
        }
        
        if (!strcmp(buffer, "m")) {
            dm.sendMsearch("upnp:rootdevice");
            dm.sendMsearch("ssdp:all");
            dm.sendMsearch("urn:dial-multiscreen-org:service:dial:1");
            continue;
        }
        
        if (!strcmp(buffer, "get")) {
            client.requestStatus();
            continue;
        }
        
        if (!strcmp(buffer, "post")) {
            string data(buffer+5);
            client.requestLaunch(data);
            continue;
        }
        
        if (!strcmp(buffer, "delete")) {
            client.requestStop();
            continue;
        }
        
        if (!strcmp(buffer, "data")) {
            string data(buffer+5);
            client.requestDialData(data);
            continue;
        }
        
        if (!strcmp(buffer, "dataUrl")) {
            string dataUrl(buffer+8);
            client.setDataUrl(dataUrl);
            printf("Set DataUrl: %s\n", dataUrl.c_str());
            continue;
        }
        
        client.setAppName(buffer);
        printf("Set AppName : %s\n", buffer);
    }
    
    dm.stop();
    
    return 0;
}
