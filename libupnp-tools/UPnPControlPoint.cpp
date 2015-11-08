#include <algorithm>
#include <liboslayer/Text.hpp>
#include <liboslayer/Logger.hpp>
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
    
    static const Logger & logger = LoggerFactory::getDefaultLogger();
    
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

    UPnPActionRequest & UPnPHttpRequestSession::getUPnPActionRequest() {
        return actionRequest;
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
    
	void UPnPSSDPMessageHandler::onNotify(const HttpHeader & header) {
		if (deviceDetection) {
			string nts = header["NTS"];
			if (Text::equalsIgnoreCase(nts, "ssdp:alive")) {
				deviceDetection->onDeviceHelloWithUrl(header["Location"], header);
			} else if (Text::equalsIgnoreCase(nts, "ssdp:byebye")) {
				deviceDetection->onDeviceByeBye(header["USN"]);
			}
            
		}
	}
	void UPnPSSDPMessageHandler::onMsearch(const HttpHeader & header, const InetAddress & remoteAddr) {
        //
	}
	void UPnPSSDPMessageHandler::onHttpResponse(const HttpHeader & header) {
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

	UPnPControlPoint::UPnPControlPoint() : httpClientThreadPool(1), deviceListener(NULL), invokeActionResponseListener(NULL) {
		ssdpHandler.setDeviceDetection(this);
		
		ssdp.addMsearchHandler(&ssdpHandler);
        ssdp.addNotifyHandler(&ssdpHandler);
        ssdp.addHttpResponseHandler(&ssdpHandler);
    
		httpClientThreadPool.setFollowRedirect(true);
		httpClientThreadPool.setHttpClientPollListener(this);
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

	int UPnPControlPoint::getMaxAgeInSecond(const string & phrase) {
		if (phrase.empty()) {
			return 0;
		}
		size_t f = phrase.find("=");
		if (f == string::npos) {
			return 0;
		}
		string name = Text::trim(phrase.substr(0, f));
		string value = Text::trim(phrase.substr(f + 1));

		if (!Text::equalsIgnoreCase(name, "max-age")) {
			return 0;
		}

		return Text::toInt(value);
	}

	void UPnPControlPoint::onHttpResponse(HttpClient<UPnPHttpRequestSession> & httpClient, const HttpHeader & responseHeader, const string & content, UPnPHttpRequestSession session) {
        logger.logd(content);
    
		if (session.getRequestType() == UPnPHttpRequestType::DEVICE_DESCRIPTION) {
			char baseUrl[1024] = {0,};
            Url & url = httpClient.getUrl();
//			snprintf(baseUrl, sizeof(baseUrl), "http://%s", url.getAddress().c_str());
            snprintf(baseUrl, sizeof(baseUrl), "%s", url.toString().c_str());
			this->onDeviceDescriptionInXml(baseUrl, content);
		} else if (session.getRequestType() == UPnPHttpRequestType::SCPD) {
			this->onScpdInXml(session.getServicePosition(), content);
        } else if (session.getRequestType() == UPnPHttpRequestType::ACTION_INVOKE) {
            SoapReader reader;
            XmlDomParser parser;
            XmlDocument doc = parser.parse(content);
            XmlNode node = reader.getActionNode(doc.getRootNode());
            string actionName = reader.getActionNameFromActionNode(node);
            UPnPActionParameters outParams = reader.getActionParametersFromActionNode(node);
            if (invokeActionResponseListener) {
                invokeActionResponseListener->onActionResponse(session.getId(), session.getUPnPActionRequest(), outParams);
            }
        }
	}
    
    void UPnPControlPoint::onError(HttpClient<UPnPHttpRequestSession> &httpClient, UPnPHttpRequestSession session) {
        logger.loge("onError()");
    }

	void UPnPControlPoint::onDeviceCacheUpdate(const HttpHeader & header) {
		string usn = header["USN"];
		Uuid uuid(usn);
		string udn = uuid.getUuid();
		if (!devicePool.hasDevice(udn)) {
			return;
		}
		string cacheControl = header["CACHE-CONTROL"];
		int maxAge = getMaxAgeInSecond(cacheControl);
		// UPnP spec: minimum 1800 (30 seconds)
		if (maxAge < 1800) {
			maxAge = 1800;
		}
		devicePool.cacheUpdate(udn, maxAge * 1000);
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
//			url.setPath(scpdurl);
            url.setRelativePath(scpdurl);
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

	void UPnPControlPoint::setFilter(const UPnPControlPointSsdpNotifyFilter & filter) {
		this->filter = filter;
	}

	vector<UPnPServicePosition> UPnPControlPoint::makeServicePositions(UPnPServicePositionMaker & maker, const UPnPDevice & device) {
    
		vector<UPnPServicePosition> servicePositions;
		const vector<UPnPService> & services = device.getServices();
        
        int i = 0;
        FOREACH_CONST_VEC(services, UPnPService, iter) {
            servicePositions.push_back(maker.makeUPnPServicePosition(i++, *iter));
        }
    
		const vector<UPnPDevice> & embeddedDevices = device.getEmbeddedDevices();
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
    
    ID_TYPE UPnPControlPoint::invokeAction(const UPnPService & service, const std::string & actionName, const UPnPActionParameters & in) {
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
        UPnPActionRequest & actionRequest = session.getUPnPActionRequest();
        actionRequest.service = service;
        actionRequest.actionName = actionName;
        actionRequest.inParameters = in;
        session.setId(gen.generate());
        httpClientThreadPool.request(url, "POST", headerFields, packet.c_str(), packet.length(), session);
        return session.getId();
    }
}
