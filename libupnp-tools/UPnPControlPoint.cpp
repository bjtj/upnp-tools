#include <algorithm>
#include <liboslayer/Text.hpp>
#include "UPnPControlPoint.hpp"
#include <libhttp-server/Url.hpp>
#include "XmlDomParser.hpp"
#include "UPnPDeviceMaker.hpp"
#include "UPnPServiceMaker.hpp"
#include "Uuid.hpp"
#include "macros.hpp"
#include "SoapWriter.hpp"
#include "SoapReader.hpp"

namespace UPNP {

	using namespace std;
    using namespace HTTP;
    using namespace OS;
	using namespace UTIL;
    using namespace XML;
    using namespace SOAP;
    
    
    /**
     * @brief invoek action response listener
     */
    
    ActionParameters::ActionParameters() {
    }
    ActionParameters::~ActionParameters() {
    }
    size_t ActionParameters::size() const {
        return params.size();
    }
    const string & ActionParameters::operator[] (const string & name) const {
        return params[name];
    }
    string & ActionParameters::operator[] (const string & name) {
        return params[name];
    }
    const NameValue & ActionParameters::operator[] (size_t index) const {
        return params[index];
    }
    NameValue & ActionParameters::operator[] (size_t index) {
        return params[index];
    }

    /**
     * @brief invoek action session
     */
    
    InvokeActionSession::InvokeActionSession() {
        
    }
    InvokeActionSession::~InvokeActionSession() {
        
    }
    UPnPService & InvokeActionSession::getUPnPService() {
        return service;
    }
    std::string & InvokeActionSession::getActionName() {
        return actionName;
    }
    ActionParameters & InvokeActionSession::getInParameters() {
        return inParameters;
    }
    const UPnPService & InvokeActionSession::getUPnPService() const {
        return service;
    }
    const std::string & InvokeActionSession::getActionName() const {
        return actionName;
    }
    const ActionParameters & InvokeActionSession::getInParameters() const {
        return inParameters;
    }
    
    
	/**
	 *
	 */

	const int UPnPHttpRequestType::UNKNOWN = 0;
	const int UPnPHttpRequestType::DEVICE_DESCRIPTION = 1;
	const int UPnPHttpRequestType::SCPD = 2;
    const int UPnPHttpRequestType::ACTION_INVOKE = 3;

	const UPnPHttpRequestType UPnPHttpRequestType::UNKNOWN_TYPE(UPnPHttpRequestType::UNKNOWN);
	const UPnPHttpRequestType UPnPHttpRequestType::DEVICE_DESCRIPTION_TYPE(UPnPHttpRequestType::DEVICE_DESCRIPTION);
	const UPnPHttpRequestType UPnPHttpRequestType::SCPD_TYPE(UPnPHttpRequestType::SCPD);
    const UPnPHttpRequestType UPnPHttpRequestType::ACTION_INVOKE_TYPE(UPnPHttpRequestType::ACTION_INVOKE);

	UPnPHttpRequestType::UPnPHttpRequestType() : type(UNKNOWN) {
	}

	UPnPHttpRequestType::UPnPHttpRequestType(int type) : type(type) {
	}

	UPnPHttpRequestType::~UPnPHttpRequestType() {
	}

	bool UPnPHttpRequestType::operator==(const int & type) {
		return this->type == type;
	}

	bool UPnPHttpRequestType::operator==(const UPnPHttpRequestType & other) {
		return this->type == other.type;
	}


	/**
	 *
	 */

	UPnPHttpRequestSession::UPnPHttpRequestSession() : id(0), deviceDetection(NULL) {
	}

	UPnPHttpRequestSession::UPnPHttpRequestSession(const UPnPHttpRequestType & type) : deviceDetection(NULL), type(type) {
	}

	UPnPHttpRequestSession::~UPnPHttpRequestSession() {
	}
    
    void UPnPHttpRequestSession::setId(ID_TYPE id) {
        this->id = id;
    }
    
    ID_TYPE UPnPHttpRequestSession::getId() {
        return id;
    }

	void UPnPHttpRequestSession::setServicePosition(const UPnPServicePosition & servicePosition) {
		this->servicePosition = servicePosition;
	}

	UPnPServicePosition & UPnPHttpRequestSession::getServicePosition() {
		return servicePosition;
	}

	UPnPHttpRequestType UPnPHttpRequestSession::getRequestType() {
		return type;
	}

	void UPnPHttpRequestSession::setDeviceDetection(UPnPDeviceDetection * deviceDetection) {
		this->deviceDetection = deviceDetection;
	}

    InvokeActionSession & UPnPHttpRequestSession::getInvokeActionSession() {
        return invokeActionSession;
    }

	/**
	 *
	 */

	UPnPControlPointSsdpNotifyFilter::UPnPControlPointSsdpNotifyFilter() {
	}

	UPnPControlPointSsdpNotifyFilter::~UPnPControlPointSsdpNotifyFilter() {
	}

	void UPnPControlPointSsdpNotifyFilter::addFilterType(const string & type) {
		filterTypes.push_back(type);
	}

	bool UPnPControlPointSsdpNotifyFilter::filter(const string & type) {
		LOOP_VEC(filterTypes, i) {
			if (!filterTypes[i].compare(type)) {
				return true;
			}
		}
		return false;
	}

    
	/**
	 *
	 */

	UPnPSSDPMessageHandler::UPnPSSDPMessageHandler() : deviceDetection(NULL) {
	}
    
	UPnPSSDPMessageHandler::~UPnPSSDPMessageHandler() {
	}
    
	void UPnPSSDPMessageHandler::onNotify(HttpHeader & header) {
		if (deviceDetection) {
			string nts = header["NTS"];
			if (Text::equalsIgnoreCase(nts, "ssdp:alive")) {
				deviceDetection->onDeviceHelloWithUrl(header["Location"], header);
			} else if (Text::equalsIgnoreCase(nts, "ssdp:byebye")) {
				deviceDetection->onDeviceByeBye(header["USN"]);
			}
            
		}
	}
	void UPnPSSDPMessageHandler::onMsearch(HttpHeader & header) {
	}
	void UPnPSSDPMessageHandler::onHttpResponse(HttpHeader & header) {
		if (deviceDetection) {
			deviceDetection->onDeviceHelloWithUrl(header["Location"], header);
		}
	}
    
	void UPnPSSDPMessageHandler::setDeviceDetection(UPnPDeviceDetection * deviceDetection) {
		this->deviceDetection = deviceDetection;
	}



	/**
	 *
	 */

	UPnPControlPoint::UPnPControlPoint() : httpClientThreadPool(10), deviceListener(NULL), invokeActionResponseListener(NULL) {
		ssdpHandler.setDeviceDetection(this);
		ssdp.addHttpResponseHandler(&ssdpHandler);
		ssdp.addNotifyHandler(&ssdpHandler);
		ssdp.addMsearchHandler(&ssdpHandler);
    
		httpClientThreadPool.setFollowRedirect(true);
		httpClientThreadPool.setHttpResponseHandler(this);
	}

	UPnPControlPoint::~UPnPControlPoint() {
	}

	void UPnPControlPoint::start() {
		ssdp.start();
		httpClientThreadPool.start();
	}

	void UPnPControlPoint::startAsync() {
		ssdp.startAsync();
		httpClientThreadPool.start();
	}

	void UPnPControlPoint::poll(unsigned long timeout) {
		ssdp.poll(timeout);
	}

	void UPnPControlPoint::stop() {
		ssdp.stop();
		httpClientThreadPool.stop();
	}

	void UPnPControlPoint::sendMsearch(string searchType) {
		ssdp.sendMsearch(searchType);
	}

	void UPnPControlPoint::onDeviceCacheUpdate(const HttpHeader & header) {
		string usn = header["USN"];
		Uuid uuid(usn);
		if (!devicePool.hasDevice(uuid.getUuid())) {
			return;
		}
		header["CACHE-CONTROL"];
	}

	void UPnPControlPoint::onDeviceHelloWithUrl(const string & url, const HttpHeader & header) {
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
		httpClientThreadPool.request(Url(url), "GET", StringMap(), NULL, 0, UPnPHttpRequestSession(UPnPHttpRequestType::DEVICE_DESCRIPTION_TYPE));
	}

	void UPnPControlPoint::onDeviceDescriptionInXml(string baseUrl, string xmlDoc) {
		XmlDomParser parser;
		UPnPDevice device = UPnPDeviceMaker::makeDeviceFromDeviceDescription(baseUrl, parser.parse(xmlDoc));
    
		devicePool.updateDevice(device);
    
		UPnPServicePositionMaker maker(device.getUdn());
		vector<UPnPServicePosition> servicePositions = makeServicePositions(maker, device);

		LOOP_VEC(servicePositions, i) {
			UPnPServicePosition & sp = servicePositions[i];
			string scpdurl = sp.getScpdUrl();
			Url url = Url(baseUrl);
			url.setPath(scpdurl);
			UPnPHttpRequestSession session(UPnPHttpRequestType::SCPD_TYPE);
			session.setServicePosition(sp);
			httpClientThreadPool.request(url, "GET", StringMap(), NULL, 0, session);
		}
	}

	void UPnPControlPoint::onScpdInXml(const UPnPServicePosition & servicePosition, string xmlDoc) {
		XmlDomParser parser;
		Scpd scpd = UPnPServiceMaker::makeScpdFromXmlDocument("", parser.parse(xmlDoc));
    
		devicePool.bindScpd(servicePosition, scpd);

		if (deviceListener) {
			UPnPDevice device = devicePool.getDevice(servicePosition.getUdn());
			if (device.complete()) {
				deviceListener->onDeviceAdded(*this, device);
			}
		}
	}

	void UPnPControlPoint::onDeviceByeBye(string udn) {
		if (!devicePool.hasDevice(udn)) {
			return;
		}
        
        if (deviceListener && devicePool.getDevice(udn).complete()) {
            deviceListener->onDeviceRemoved(*this, devicePool.getDevice(udn));
        }
        
		devicePool.removeDevice(udn);
	}

	void UPnPControlPoint::onResponse(HttpClient<UPnPHttpRequestSession> & httpClient, HttpHeader & responseHeader, OS::Socket & socket, UPnPHttpRequestSession session) {
    
		if (session.getRequestType() == UPnPHttpRequestType::DEVICE_DESCRIPTION) {
			string dump = HttpResponseDump::dump(responseHeader, socket);
			char baseUrl[1024] = {0,};
			snprintf(baseUrl, sizeof(baseUrl), "http://%s:%d", socket.getHost(), socket.getPort());
			this->onDeviceDescriptionInXml(baseUrl, dump);
		} else if (session.getRequestType() == UPnPHttpRequestType::SCPD) {
			string dump = HttpResponseDump::dump(responseHeader, socket);
			this->onScpdInXml(session.getServicePosition(), dump);
        } else if (session.getRequestType() == UPnPHttpRequestType::ACTION_INVOKE) {
			string dump = HttpResponseDump::dump(responseHeader, socket);
            SoapReader reader;
            XmlDomParser parser;
            XmlDocument doc = parser.parse(dump);
            XmlNode node = reader.getActionNode(doc.getRootNode());
            string actionName = reader.getActionNameFromActionNode(node);
            ActionParameters outParams = reader.getActionParametersFromActionNode(node);
            if (invokeActionResponseListener) {
                invokeActionResponseListener->onActionResponse(session.getId(), session.getInvokeActionSession(), outParams);
            }
        }
	}

	void UPnPControlPoint::setFilter(const UPnPControlPointSsdpNotifyFilter & filter) {
		this->filter = filter;
	}

	vector<UPnPServicePosition> UPnPControlPoint::makeServicePositions(UPnPServicePositionMaker & maker, const UPnPDevice & device) {
    
		vector<UPnPServicePosition> servicePositions;
		vector<UPnPService> & services = device.getServices();
		LOOP_VEC(services, i) {
			servicePositions.push_back(maker.makeUPnPServicePosition(i, services[i]));
		}
    
		vector<UPnPDevice> & embeddedDevices = device.getEmbeddedDevices();
		LOOP_VEC(embeddedDevices, i) {
			maker.enter();
			maker.setDeviceIndex(i);
			vector<UPnPServicePosition> append = makeServicePositions(maker, embeddedDevices[i]);
			servicePositions.insert(servicePositions.end(), append.begin(), append.end());
			maker.leave();
		}
    
		return servicePositions;
	}

	void UPnPControlPoint::setDeviceAddRemoveListener(DeviceAddRemoveListener * deviceListener) {
		this->deviceListener = deviceListener;
	}
    
    void UPnPControlPoint::setInvokeActionResponseListener(InvokeActionResponseListener * invokeActionResponseListener) {
        this->invokeActionResponseListener = invokeActionResponseListener;
    }
    
    ID_TYPE UPnPControlPoint::invokeAction(const UPnPService & service, const std::string & actionName, const ActionParameters & in) {
        Url url(service.getBaseUrl());
		url.setPath(service["controlURL"]);
        StringMap headerFields;
        headerFields["SOAPACTION"] = service.getServiceType() + "#" + actionName;
        SoapWriter writer;
        writer.setSoapAction(service.getServiceType(), actionName);
		LOOP_VEC(in, i) {
			const NameValue & nv = in[i];
			writer.setArgument(nv.getName(), nv.getValue());
		}
        string packet = writer.toString();
		UPnPHttpRequestSession session(UPnPHttpRequestType::ACTION_INVOKE);
        InvokeActionSession & as = session.getInvokeActionSession();
        as.getUPnPService() = service;
        as.getActionName() = actionName;
        as.getInParameters() = in;
        session.setId(gen.generate());
        httpClientThreadPool.request(url, "POST", headerFields, packet.c_str(), packet.length(), session);
        return session.getId();
    }
}
