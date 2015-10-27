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
        
        string location = header["Location"];
        if (!location.empty()) {
            cp.ssdpDeviceFound(location);
        }
	}
    
    BuildTarget::BuildTarget() : cp(NULL), deviceFrame(NULL), targetService(NULL) {
    }
    
    BuildTarget::BuildTarget(UPnPControlPoint * cp) : cp(cp), deviceFrame(NULL), targetService(NULL) {
    }
		
    BuildTarget::~BuildTarget() {
    }

	bool BuildTarget::empty() {
		return deviceFrame == NULL && targetService == NULL;
	}
    
    bool BuildTarget::hasTagetService() {
        return targetService != NULL;
    }
	UPnPDevice * BuildTarget::getDeviceFrame() {
		return deviceFrame;
	}
	void BuildTarget::setDeviceFrame(UPnPDevice * deviceFrame) {
		this->deviceFrame = deviceFrame;
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

	UPnPControlPoint * BuildTarget::getControlPoint() {
		return cp;
	}
    
    
    ControlPointHttpResponseHandler::ControlPointHttpResponseHandler(UPnPControlPoint & cp) : cp(cp) {
        
    }
    ControlPointHttpResponseHandler::~ControlPointHttpResponseHandler() {
        
    }
    
    void ControlPointHttpResponseHandler::onResponse(HttpClient<BuildTarget> & client,
                                                     HttpHeader & responseHeader,
                                                     Socket & socket,
                                                     BuildTarget buildTarget) {


		HttpResponseDump dump;
		string xmlDump = dump.dump(responseHeader, socket);
		XmlDomParser parser;
		XmlDocument xmlDoc = parser.parse(xmlDump);
		UPnPControlPoint * cp = buildTarget.getControlPoint();

		if (buildTarget.empty()) {
			Url url(string("http://") + socket.getHost() + ":" + Text::toString(socket.getPort()));
			cp->handleDeviceDescrition(xmlDoc, url);
		} else {
			cp->handleScpd(buildTarget, xmlDoc);
		}
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
        httpClient.request(url, get, NULL, 0, BuildTarget(this));
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
        devices.erase(udn);
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
	UPnPDevice * UPnPControlPoint::registerDeviceFrame(UPnPDevice & deviceFrame) {
		string udn = deviceFrame.getUdn();
		deviceFrames[udn] = deviceFrame;
		return &(deviceFrames[udn]);
	}

	void UPnPControlPoint::removeDeviceFrame(const string & udn) {
		deviceFrames.erase(udn);
	}

	vector<BuildTarget> UPnPControlPoint::makeBuildTargets(UPnPDevice * device, Url & url) {

		vector<BuildTarget> buildTargets;
		vector<UPnPService> services = device->getServices();
		for (size_t i = 0; i < services.size(); i++) {
			UPnPService & service = services[i];
			BuildTarget target(this);
			target.setDeviceFrame(device);
			target.setTargetService(&service);
			Url scpdUrl = url;
			scpdUrl.setPath(service["SCPDURL"]);
			target.setUrl(scpdUrl);
			buildTargets.push_back(target);
		}

		vector<UPnPDevice> & embeddedDevices = device->getEmbeddedDevices();
		for (size_t i = 0; i < embeddedDevices.size(); i++) {
			UPnPDevice & embeddedDevice = embeddedDevices[i];
			vector<BuildTarget> append = makeBuildTargets(&embeddedDevice, url);
			buildTargets.insert(buildTargets.end(), append.begin(), append.end());
		}
		return buildTargets;
	}

	void UPnPControlPoint::registerBuildTargets(const string & udn, vector<BuildTarget> & buildTargets) {
		LOOP_VEC(buildTargets, i) {
			registerBuildTarget(udn, buildTargets[i]);
		}
	}

	void UPnPControlPoint::registerBuildTarget(const string & udn, BuildTarget & buildTarget) {
		string get = "GET";
		buildTargetTable[udn].push_back(buildTarget);
		httpClient.request(buildTarget.getUrl(), get, NULL, 0, buildTarget);
	}

	void UPnPControlPoint::unregisterBuildTarget(BuildTarget & buildTarget) {
		string udn = buildTarget.getDeviceFrame()->getUdn();
		vector<BuildTarget> & targets = buildTargetTable[udn];
		LOOP_VEC(targets, i) {
			if (targets[i].getTargetService() == buildTarget.getTargetService()) {
				targets.erase(targets.begin() + i);
				break;
			}
		}
		if (targets.size() == 0) {
			UPnPDevice * device = buildTarget.getDeviceFrame();
			addDevice(*device);
			buildTargetTable.erase(udn);
		}
	}

	void UPnPControlPoint::setScpdToUPnPService(UPnPService & targetService, XmlDocument & doc) {
		UPnPServiceMaker maker;
		Scpd scpd = maker.makeScpdWithXmlDocument(targetService.getServiceType(), doc);
		targetService.setScpd(scpd);
	}

	void UPnPControlPoint::handleDeviceDescrition(XmlDocument & doc, Url & url) {

		/*deviceListLock.wait();*/
		UPnPDevice device = makeUPnPDeviceFrame(doc);
		if (hasDevice(device.getUdn())) {
			return;
		}

		UPnPDevice * deviceFrame = registerDeviceFrame(device);
		if (deviceFrame) {
			string udn = deviceFrame->getUdn();
			vector<BuildTarget> buildTargets = makeBuildTargets(deviceFrame, url);
			if (buildTargets.size() > 0) {
				registerBuildTargets(udn, buildTargets);
			} else {
				addDevice(*deviceFrame);
				removeDeviceFrame(udn);
			}
		}
		/*deviceListLock.post();*/
	}

	void UPnPControlPoint::handleScpd(BuildTarget & buildTarget, XML::XmlDocument & doc) {
		/*deviceListLock.wait();*/
		UPnPService * service = buildTarget.getTargetService();
		setScpdToUPnPService(*service, doc);
		unregisterBuildTarget(buildTarget);
		/*deviceListLock.post();*/
	}
}
