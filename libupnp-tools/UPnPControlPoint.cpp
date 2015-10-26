#include "UPnPControlPoint.hpp"
#include <libhttp-server/Url.hpp>
#include "XmlDomParser.hpp"
#include "UPnPDeviceMaker.hpp"

namespace UPNP {

	using namespace std;
    using namespace HTTP;
    using namespace OS;
    using namespace XML;

	/**
	 * @brief ssdp handler
	 */
    ControlPointSSDPHandler::ControlPointSSDPHandler(UPnPControlPoint & cp) : cp(cp) {
	}
	ControlPointSSDPHandler::~ControlPointSSDPHandler() {
	}
	void ControlPointSSDPHandler::onNotify(HTTP::HttpHeader & header) {
		// request device description
        
        string location = header["Location"];
        if (!location.empty()) {
            cp.ssdpDeviceFound(location);
        }
	}
    
    
    
    ControlPointHttpResponseHandler::ControlPointHttpResponseHandler(UPnPControlPoint & cp) : cp(cp) {
        
    }
    ControlPointHttpResponseHandler::~ControlPointHttpResponseHandler() {
        
    }
    
    void ControlPointHttpResponseHandler::onResponse(HttpClient & client,
                                                     HttpHeader & responseHeader,
                                                     Socket & socket) {
        HttpResponseDump dump;
        string deviceDescription = dump.dump(responseHeader, socket);
        cp.addDevice(deviceDescription);
    }
    
    

	/**
	 * @brief upnp control point
	 */
	UPnPControlPoint::UPnPControlPoint(int port, string searchTarget) : 
		httpClient(5), searchTarget(searchTarget), ssdpHandler(*this), httpServer(port), listener(NULL),
        httpResponseHandler(*this), deviceListLock(1) {
            
            httpClient.setHttpResponseHandler(&httpResponseHandler);
            httpClient.setFollowRedirect(true);
	}

	UPnPControlPoint::~UPnPControlPoint() {
	}

	void UPnPControlPoint::startAsync() {
        
        httpClient.start();

		ssdpServer.addNotifyHandler(&ssdpHandler);
		
		ssdpServer.startAsync();
		httpServer.startAsync();
	}

	void UPnPControlPoint::stop() {
        
        httpClient.stop();
        
		ssdpServer.stop();
		httpServer.stop();
	}

	bool UPnPControlPoint::isRunning() {
		return false;
	}

	void UPnPControlPoint::sendMsearch() {
		ssdpServer.sendMsearch(searchTarget);
	}

	vector<UPnPDevice> UPnPControlPoint::getDeviceList() {
		return devices;
	}

	UPnPDevice UPnPControlPoint::getDevice(string udn) {
		
        UPnPDevice ret;
        
        deviceListLock.wait();
		for (vector<UPnPDevice>::iterator iter = devices.begin(); 
			 iter != devices.end();
			 iter++) {
			
			UPnPDevice & device = *iter;
			if (!device.getUdn().compare(udn)) {
                ret = device;
                break;
			}
		}
        deviceListLock.post();
        
		return ret;
	}
	
	void UPnPControlPoint::removeAllDevices() {
		devices.clear();
	}
	
	void UPnPControlPoint::setSearchTarget(string searchTarget) {
		this->searchTarget = searchTarget;
	}
	
	std::string UPnPControlPoint::getSearchTaget() {
		return searchTarget;
	}
	
	void UPnPControlPoint::setOnDeviceAddRemoveListener(OnDeviceAddRemoveListener * listener) {
		this->listener = listener;
	}
    
    void UPnPControlPoint::ssdpDeviceFound(const string & urlString) {
        
        Url url(urlString);
        string get = "GET";
        httpClient.request(url, get, NULL, 0);
    }
    
    void UPnPControlPoint::addDevice(const string & deviceDescription) {
        UPnPDevice device = makeUPnPDevice(deviceDescription);
        addDevice(device);
    }
    
    void UPnPControlPoint::addDevice(UPnPDevice & device) {
        deviceListLock.wait();
        devices.push_back(device);
		if (listener) {
			listener->onDeviceAdd(device);
		}
        deviceListLock.post();
    }
    
    void UPnPControlPoint::removeDevice(const string & udn) {
        deviceListLock.wait();
        for (vector<UPnPDevice>::iterator iter = devices.begin(); iter != devices.end(); iter++) {
            
            UPnPDevice & device = *iter;
            if (!device.getUdn().compare(udn)) {
                devices.erase(iter);
                break;
            }
        }
        deviceListLock.post();
    }
    
    UPnPDevice UPnPControlPoint::makeUPnPDevice(const string & deviceDescription) {
        UPnPDevice device;
        XmlDomParser parser;
        XmlDocument doc = parser.parse(deviceDescription);
		UPnPDeviceMaker maker;
		device = maker.makeDeviceWithDeviceDescription(doc);
        return device;
    }
}
