#include <algorithm>
#include <liboslayer/Text.hpp>
#include <liboslayer/Measurement.hpp>
#include <liboslayer/Logger.hpp>
#include "UPnPControlPoint.hpp"
#include <libhttp-server/Url.hpp>
#include <libhttp-server/FixedTransfer.hpp>
#include "XmlDomParser.hpp"
#include "XmlNodeFinder.hpp"
#include "UPnPDeviceMaker.hpp"
#include "UPnPServiceMaker.hpp"
#include "Uuid.hpp"
#include "macros.hpp"
#include "SoapWriter.hpp"
#include "SoapReader.hpp"
#include "NetworkUtil.hpp"

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
	 * @brief UPnPSSDPPacketHandler
	 */
    
    UPnPSSDPPacketHandler::UPnPSSDPPacketHandler() : deviceDetection(NULL) {
    }
    UPnPSSDPPacketHandler::~UPnPSSDPPacketHandler() {
    }
    void UPnPSSDPPacketHandler::onNotify(SSDP::SSDPHeader & header) {
        if (deviceDetection) {
            if (header.isNotifyAlive()) {
                deviceDetection->onDeviceHelloWithUrl(header.getLocation(), header);
            } else if (header.isNotifyByebye()) {
                deviceDetection->onDeviceByeBye(header.getUsn());
            }
        }
    }
    void UPnPSSDPPacketHandler::onMsearch(SSDP::SSDPHeader & header) {
    }
    void UPnPSSDPPacketHandler::onMsearchResponse(SSDP::SSDPHeader & header) {
        if (deviceDetection) {
            deviceDetection->onDeviceHelloWithUrl(header.getLocation(), header);
        }
    }
    void UPnPSSDPPacketHandler::setDeviceDetection(UPnPDeviceDetection * deviceDetection) {
        this->deviceDetection = deviceDetection;
    }
    
    /**
     * @brief UPnPEventSubscriptions
     */
    
    UPnPEventSubscriptions::UPnPEventSubscriptions() {
        
    }
    UPnPEventSubscriptions::~UPnPEventSubscriptions() {
    }
    
    void UPnPEventSubscriptions::subscribe(const string & sid, UPnPService & service) {
        subscriptions[sid] = service;
    }
    void UPnPEventSubscriptions::unsubscribe(const string & sid) {
        subscriptions.erase(sid);
    }
    UPnPService UPnPEventSubscriptions::getService(const string & sid) {
        return subscriptions[sid];
    }

    /**
     *
     */
    
    UPnPControlPointThread::UPnPControlPointThread(UPnPControlPoint * cp) : cp(cp) {
        
    }
    UPnPControlPointThread::~UPnPControlPointThread() {
        
    }
    
    void UPnPControlPointThread::run() {
        while (!interrupted()) {
            cp->poll(100);
        }
    }

	/**
	 * @brief UPnPControlPoint
	 */

	UPnPControlPoint::UPnPControlPoint(int port) : TimerEvent(false), deviceListener(NULL), invokeActionResponseListener(NULL), thread(NULL), anotherHttpClientThreadPool(5), eventListener(NULL), httpServer(port), running(false) {
        
        ssdpPacketHandler.setDeviceDetection(this);
        ssdpServer.setSSDPPacketHandler(&ssdpPacketHandler);
        registerPollee(&timer);
        scheduleRepeatableRelativeTimer(0, -1, Timer::SECOND);
        anotherHttpClientThreadPool.setOnRequestCompleteListener(this);
        httpServer.registerRequestHandler("/notify", this);
	}

	UPnPControlPoint::~UPnPControlPoint() {
		printf("destruction of UPnPControlPoint\n");
        stop();
	}
    
    void UPnPControlPoint::onTimerTriggered() {
		// TODO: Check UPnPDevice cache and remove outdated devices
        // TODO: renew subscription
    }

	void UPnPControlPoint::start() {
        if (!running) {
            ssdpServer.start();
            timer.start();
            timer.setTimerEvent(this);
            anotherHttpClientThreadPool.start();
            httpServer.start();
            running = true;
        }
        
	}

	void UPnPControlPoint::startAsync() {
        if (!running) {
            start();
            httpServer.startAsync();
            startThread();
            running = true;
        }
	}

	void UPnPControlPoint::stop() {
        if (running) {
            stopThread();
            timer.stop();
            ssdpServer.stop();
            anotherHttpClientThreadPool.stop();
            httpServer.stop();
            running = false;
        }
	}
    
    void UPnPControlPoint::startThread() {
        if (!thread) {
            thread = new UPnPControlPointThread(this);
            thread->start();
        }
    }
    void UPnPControlPoint::stopThread() {
        if (thread) {
            thread->interrupt();
            thread->join();
            delete thread;
            thread = NULL;
        }
    }

	void UPnPControlPoint::poll(unsigned long timeout) {
        ssdpServer.poll(timeout);
		anotherHttpClientThreadPool.collectUnflaggedThreads();
	}

	void UPnPControlPoint::sendMsearch(string searchType) {
		ssdpServer.sendMsearchAndGather(searchType, 5);
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

	void UPnPControlPoint::sendHttpRequest(const Url & url, const string & method, const StringMap & additionalFields, const string & content, UserData * userData) {
		anotherHttpClientThreadPool.collectUnflaggedThreads();
		anotherHttpClientThreadPool.setRequest(url, method, additionalFields, new FixedTransfer(content.c_str(), content.length()), AutoRef<UserData>(userData));
    }
    
    void UPnPControlPoint::onRequestComplete(Url & url, HttpResponse & response, const string & content, UserData * userData) {
        
        if (!userData) {
            return;
        }
        
		UPnPHttpRequestSession & session = (UPnPHttpRequestSession &)*userData;
        
		if (session.getRequestType() == UPnPHttpRequestType::DEVICE_DESCRIPTION) {
            
			onDeviceDescriptionInXml(url.toString(), content);
            
		} else if (session.getRequestType() == UPnPHttpRequestType::SCPD) {
            
			onScpdInXml(session.getServicePosition(), content);
            
        } else if (session.getRequestType() == UPnPHttpRequestType::ACTION_INVOKE) {
            
            SoapReader reader;
            XmlDomParser parser;
            XmlDocument doc = parser.parse(content);

			UPnPActionResponse actionResponse;
			HttpResponseHeader header = response.getHeader();
			actionResponse.setResult(UPnPActionResult(true, header.getStatusCode(), header.getMessage()));

			try {

				XmlNode node = reader.getActionNode(doc.getRootNode());
				string actionName = reader.getActionNameFromActionNode(node);
				UPnPActionParameters outParams = reader.getActionParametersFromActionNode(node);
				actionResponse.setParameters(outParams);

			} catch (Exception e) {
				logger.loge(e.getMessage());
			}

            if (invokeActionResponseListener) {
                invokeActionResponseListener->onActionResponse(session.getId(), session.getUPnPActionRequest(), actionResponse);
            }
        }

    }
    void UPnPControlPoint::onRequestError(Exception & e, Url & url, UserData * userData) {
		logger.loge("Http Request Error/url: " + url.toString() + "/e: " + e.getMessage());
    }

	void UPnPControlPoint::onDeviceCacheUpdate(const HttpHeader & header) {
		string usn = header.getHeaderFieldIgnoreCase("USN");
		Uuid uuid(usn);
		string udn = uuid.getUuid();
		if (!devicePool.hasDevice(udn)) {
			return;
		}
		string cacheControl = header.getHeaderFieldIgnoreCase("CACHE-CONTROL");
		int maxAge = getMaxAgeInSecond(cacheControl);

		/* UPnP spec: minimum 1800 (30 seconds) */
		if (maxAge < 1800) {
			maxAge = 1800;
		}
		devicePool.cacheUpdate(udn, maxAge * 1000);
	}

	void UPnPControlPoint::onDeviceHelloWithUrl(const string & url, const HttpHeader & header) {
		string usn = header.getHeaderFieldIgnoreCase("USN");
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
		sendHttpRequest(Url(url), "GET", StringMap(), "", new UPnPHttpRequestSession(UPnPHttpRequestType::DEVICE_DESCRIPTION_TYPE));
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
            url.setRelativePath(scpdurl);
			UPnPHttpRequestSession * session = new UPnPHttpRequestSession(UPnPHttpRequestType::SCPD_TYPE);
			session->setServicePosition(sp);
			sendHttpRequest(url, "GET", StringMap(), "", session);
		}
	}

	void UPnPControlPoint::onScpdInXml(const UPnPServicePosition & servicePosition, string xmlDoc) {

		Measurement m;

		XmlDomParser parser;
        /* TODO: performence check in Windows platform */
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
        
		UPnPDevice & device = devicePool.getDevice(udn);

		if (deviceListener && device.valid()) {
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
        
        if (actionName.empty()) {
            throw IllegalArgumentException("action name required", -1, 0);
        }
        
        Url url(service.getBaseUrl());
		url.setPath(service.getProperty("controlURL"));
        StringMap headerFields;
        headerFields["SOAPACTION"] = service.getServiceType() + "#" + actionName;
        SoapWriter writer;
        writer.setSoapAction(service.getServiceType(), actionName);

		vector<string> names = in.getParameterNames();
		LOOP_VEC(names, i) {
			string & name = names[i];
			string value = in.getParameter(name);
			writer.setArgument(name, value);
		}
        string packet = writer.toString();
		UPnPHttpRequestSession * session = new UPnPHttpRequestSession(UPnPHttpRequestType::ACTION_INVOKE);
        UPnPActionRequest & actionRequest = session->getUPnPActionRequest();
        actionRequest.setService(service);
        actionRequest.setActionName(actionName);
		actionRequest.setParameters(in);
        session->setId(gen.generate());
		sendHttpRequest(url, "POST", headerFields, packet, session);

        return session->getId();
    }
    
    void UPnPControlPoint::subscribe(UPnPService & service) {
        // TODO: test this function
        
        Url url;
        
        url.setUrl(service.getBaseUrl());
        url.setRelativePath(service.getEventSubscribeUrl());
        
        StringMap fields;
        fields["NT"] = "upnp:event";
        InetAddress addr = NetworkUtil::selectDefaultAddress();
        addr.setPort(httpServer.getPort());
        fields["CALLBACK"] = "<http://" + addr.getHost() + ":" + Text::toString(addr.getPort()) + "/notify" + ">";
        fields["TIMEOUT"] = "Second-300";
        
        sendHttpRequest(url, "SUBSCRIBE", fields, "", NULL);
    }
    
    void UPnPControlPoint::unsubscribe(UPnPService & service) {
        // TODO: implement it
    }
    
    void UPnPControlPoint::setEventListener(UPnPEventListener * eventListener) {
        this->eventListener = eventListener;
    }
    
    void UPnPControlPoint::onHttpRequestHeaderCompleted(HttpRequest & request, HttpResponse & response) {
        
        response.setStatusCode(404);
        response.getHeader().setConnection("close");
        
        logger.logv(request.getPath());
    }
    
    void UPnPControlPoint::onHttpRequestContent(HttpRequest & request, HttpResponse & response, Packet & packet) {
    }
    
    void UPnPControlPoint::onHttpRequestContentCompleted(HttpRequest &request, HttpResponse &response) {
        
        AutoRef<DataTransfer> transfer = request.getTransfer();
        if (!transfer.empty()) {
            string content = transfer->getString();
            
            string sid = request.getHeaderField("SID");
            Uuid uuid(sid);
            
            string uuidOfSid = uuid.getUuid();
            
            UPnPService service = subscriptions.getService(uuidOfSid);
            
            LinkedStringMap values = parsePropertySetXmlString(content);
            
            if (eventListener) {
                eventListener->onEvent(service, values);
            }
        }
    }
    
    LinkedStringMap UPnPControlPoint::parsePropertySetXmlString(const string & propertySetXmlString) {
        
        XmlDomParser parser;
        XmlDocument doc = parser.parse(propertySetXmlString);
        vector<XmlNode> nodes = XmlNodeFinder::getAllNodesByTagName(doc.getRootNode(), "property");
        
        LinkedStringMap values;
        
        for (vector<XmlNode>::iterator iter = nodes.begin(); iter != nodes.end(); iter++) {
            XmlNode node = iter->getFirstElement();
            string variableName = node.getTagName();
            string changedValue = node.getFirstContent();
            values[variableName] = changedValue;
        }
        
        return values;
    }
}
