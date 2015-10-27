#include <algorithm>
#include <liboslayer/Text.hpp>
#include "UPnPControlPoint.hpp"
#include <libhttp-server/Url.hpp>
#include "XmlDomParser.hpp"
#include "UPnPDeviceMaker.hpp"
#include "UPnPServiceMaker.hpp"
#include "macros.hpp"

namespace UPNP {

	using namespace std;
    using namespace HTTP;
    using namespace OS;
	using namespace UTIL;
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
        
        string nts = header["NTS"];
        
        if (Text::equalsIgnoreCase(nts, "ssdp:alive")) {
            string location = header["Location"];
            if (!location.empty()) {
                cp.ssdpDeviceFound(location);
            }
        }
        
        if (Text::equalsIgnoreCase(nts, "ssdp:byebye")) {
            string usn = header["USN"];
            if (!usn.empty()) {
                cp.ssdpDeviceRemoved(usn);
            }
        }
        
        
	}
        
    BuildTarget::BuildTarget(UPnPControlPoint & cp) : cp(cp), device(NULL), targetService(NULL) {
    }
		
    BuildTarget::~BuildTarget() {
    }
    
    bool BuildTarget::hasTagetService() {
        return targetService != NULL;
    }
	UPnPDevice * BuildTarget::getDevice() {
		return device;
	}
	void BuildTarget::setDevice(UPnPDevice * device) {
		this->device = device;
	}
    UPnPService * BuildTarget::getTargetService() {
        return targetService;
    }
	void BuildTarget::setTargetService(UPnPService * targetService) {
        this->targetService = targetService;
    }
	Url & BuildTarget::getUrl() {
		return url;
	}
	void BuildTarget::setUrl(Url & url) {
		this->url = url;
	}
    
    void BuildTarget::setUdn(const string & udn) {
        this->udn = udn;
    }
    
    string BuildTarget::getUdn() {
        return udn;
    }

	UPnPControlPoint & BuildTarget::getControlPoint() {
		return cp;
	}
    
    
    ControlPointHttpResponseHandler::ControlPointHttpResponseHandler(UPnPControlPoint & cp) : cp(cp) {
        
    }
    ControlPointHttpResponseHandler::~ControlPointHttpResponseHandler() {
        
    }
    
    void ControlPointHttpResponseHandler::onResponse(HttpClient<BuildTarget*> & client,
                                                     HttpHeader & responseHeader,
                                                     Socket & socket,
                                                     BuildTarget * buildTarget) {


		HttpResponseDump dump;
		string xmlDump = dump.dump(responseHeader, socket);
		XmlDomParser parser;
		XmlDocument xmlDoc = parser.parse(xmlDump);
		UPnPControlPoint & cp = buildTarget->getControlPoint();

		if (buildTarget->getTargetService() == NULL) {
			Url url(string("http://") + socket.getHost() + ":" + Text::toString(socket.getPort()));
			cp.handleDeviceDescrition(xmlDoc, url);
            delete buildTarget;
		} else {
			cp.handleScpd(buildTarget, xmlDoc);
		}
    }
    
	/**
	 * @brief upnp control point
	 */
	UPnPControlPoint::UPnPControlPoint(int port, string searchTarget) : 
		httpClient(5), searchTarget(searchTarget), ssdpHandler(*this), httpServer(port), listener(NULL),
        httpResponseHandler(*this), deviceListLock(1), buildTargetLock(1) {
            
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
        
        LOOP_MAP(buildTargetTable, string, vector<BuildTarget*>, iter) {
            vector<BuildTarget*> targets = iter->second;
            UPnPDevice * device = NULL;
            LOOP_VEC(targets, i) {
                device = targets[i]->getDevice();
                delete targets[i];
            }
            if (device) {
                delete device;
            }
        }
	}

	bool UPnPControlPoint::isRunning() {
		return false;
	}

	void UPnPControlPoint::sendMsearch() {
		ssdpServer.sendMsearch(searchTarget);
	}

	vector<UPnPDevice> UPnPControlPoint::getDeviceList() {
		vector<UPnPDevice> list;
		deviceListLock.wait();
		for (map<string, UPnPDevice>::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			list.push_back(iter->second);
		}
		deviceListLock.post();
		return list;
	}

	UPnPDevice UPnPControlPoint::getDevice(string udn) {
        UPnPDevice ret = devices[udn];
		return ret;
	}
	
	void UPnPControlPoint::removeAllDevices() {
        deviceListLock.wait();
		devices.clear();
        deviceListLock.post();
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
        httpClient.request(url, get, NULL, 0, new BuildTarget(*this));
    }
    
    void UPnPControlPoint::ssdpDeviceRemoved(const string & udn) {
        removeDevice(udn);
    }
    
    void UPnPControlPoint::addDevice(UPnPDevice & device) {
        
        deviceListLock.wait();
        devices[device.getUdn()] = device;
		if (listener) {
			listener->onDeviceAdd(device);
		}
        deviceListLock.post();
    }
    
    void UPnPControlPoint::removeDevice(const string & udn) {
        
        deviceListLock.wait();
        if (devices.find(udn) != devices.end()) {
            if (listener) {
                listener->onDeviceRemove(devices[udn]);
            }
            devices.erase(udn);
        }
        deviceListLock.post();
    }

	bool UPnPControlPoint::hasDevice(const std::string & udn) {
		return devices.find(udn) != devices.end() || buildTargetTable.find(udn) != buildTargetTable.end();
	}

	UPnPDevice & UPnPControlPoint::getDevice(const std::string & udn) {
		return devices[udn];
	}
    
	UPnPDevice UPnPControlPoint::makeUPnPDeviceFrame(XmlDocument & doc) {
		UPnPDeviceMaker maker;
		return maker.makeDeviceWithDeviceDescription(doc);
	}
    
    BuildTarget * UPnPControlPoint::makeBuildTarget(UPnPDevice * device, UPnPService * service, Url & url) {
        BuildTarget * target = new BuildTarget(*this);
        target->setUdn(device->getUdn());
        target->setDevice(device);
        target->setTargetService(service);
        Url scpdUrl = url;
        scpdUrl.setPath((*service)["SCPDURL"]);
        target->setUrl(scpdUrl);
        return target;
    }

	vector<BuildTarget*> UPnPControlPoint::makeBuildTargets(UPnPDevice * device, Url & url) {

		vector<BuildTarget*> buildTargets;
		vector<UPnPService> & services = device->getServices();
		for (size_t i = 0; i < services.size(); i++) {
			UPnPService * service = &services[i];
			buildTargets.push_back(makeBuildTarget(device, service, url));
		}

		vector<UPnPDevice> & embeddedDevices = device->getEmbeddedDevices();
		for (size_t i = 0; i < embeddedDevices.size(); i++) {
			UPnPDevice * embeddedDevice = &embeddedDevices[i];
			vector<BuildTarget*> append = makeBuildTargets(embeddedDevice, url);
			buildTargets.insert(buildTargets.end(), append.begin(), append.end());
		}
		return buildTargets;
	}

	void UPnPControlPoint::registerBuildTargets(const string & udn, vector<BuildTarget*> & buildTargets) {
		LOOP_VEC(buildTargets, i) {
			registerBuildTarget(udn, buildTargets[i]);
		}
	}

	void UPnPControlPoint::registerBuildTarget(const string & udn, BuildTarget * buildTarget) {
		string get = "GET";
		buildTargetTable[udn].push_back(buildTarget);
		httpClient.request(buildTarget->getUrl(), get, NULL, 0, buildTarget);
	}

	void UPnPControlPoint::unregisterBuildTarget(BuildTarget * buildTarget) {
		string udn = buildTarget->getUdn();
		vector<BuildTarget*> & targets = buildTargetTable[udn];
        
        targets.erase(std::remove(targets.begin(), targets.end(), buildTarget), targets.end());

		if (targets.size() == 0) {
			UPnPDevice * device = buildTarget->getDevice()->getRootDevice();
            UPnPDevice copiedDevice = device->copy();
			addDevice(copiedDevice);
			buildTargetTable.erase(udn);
            
            delete device;
		}
        
        delete buildTarget;
	}

	void UPnPControlPoint::setScpdToUPnPService(UPnPService * targetService, XmlDocument & doc) {
        
		UPnPServiceMaker maker;
		Scpd scpd = maker.makeScpdWithXmlDocument(targetService->getServiceType(), doc);
		targetService->setScpd(scpd);
	}

	void UPnPControlPoint::handleDeviceDescrition(XmlDocument & doc, Url & url) {

        buildTargetLock.wait();

		UPnPDevice device = makeUPnPDeviceFrame(doc);
		if (!device.getUdn().empty() && !hasDevice(device.getUdn())) {
            string udn = device.getUdn();
            vector<BuildTarget*> buildTargets = makeBuildTargets(new UPnPDevice(device), url);
            if (buildTargets.size() > 0) {
                registerBuildTargets(udn, buildTargets);
            } else {
                addDevice(device);
            }
		}
        buildTargetLock.post();
	}

	void UPnPControlPoint::handleScpd(BuildTarget * buildTarget, XML::XmlDocument & doc) {
        
        buildTargetLock.wait();
        
		UPnPService * service = buildTarget->getTargetService();
		setScpdToUPnPService(service, doc);
		unregisterBuildTarget(buildTarget);

        buildTargetLock.post();
	}
}
