#include <algorithm>
#include <liboslayer/Text.hpp>
#include "UPnPControlPoint.hpp"
#include <libhttp-server/Url.hpp>
#include "XmlDomParser.hpp"
#include "UPnPDeviceMaker.hpp"
#include "UPnPServiceMaker.hpp"
#include "Uuid.hpp"
#include "macros.hpp"

namespace UPNP {

	using namespace std;
    using namespace HTTP;
    using namespace OS;
	using namespace UTIL;
    using namespace XML;

	/**
	 *
	 */

	const int UPnPHttpRequestType::UNKNOWN = 0;
	const int UPnPHttpRequestType::DEVICE_DESCRIPTION = 1;
	const int UPnPHttpRequestType::SCPD = 2;

	const UPnPHttpRequestType UPnPHttpRequestType::UNKNOWN_TYPE(UPnPHttpRequestType::UNKNOWN);
	const UPnPHttpRequestType UPnPHttpRequestType::DEVICE_DESCRIPTION_TYPE(UPnPHttpRequestType::DEVICE_DESCRIPTION);
	const UPnPHttpRequestType UPnPHttpRequestType::SCPD_TYPE(UPnPHttpRequestType::SCPD);

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

	UPnPHttpRequestSession::UPnPHttpRequestSession() : deviceDetection(NULL) {
	}

	UPnPHttpRequestSession::UPnPHttpRequestSession(const UPnPHttpRequestType & type) : deviceDetection(NULL), type(type) {
	}

	UPnPHttpRequestSession::~UPnPHttpRequestSession() {
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

	UPnPControlPoint::UPnPControlPoint() : httpClientThreadPool(10), deviceListener(NULL) {
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
		httpClientThreadPool.request(Url(url), "GET", NULL, 0, UPnPHttpRequestSession(UPnPHttpRequestType::DEVICE_DESCRIPTION_TYPE));
	}

	void UPnPControlPoint::onDeviceDescriptionInXml(string baseUrl, string xmlDoc) {
		XmlDomParser parser;
		UPnPDevice device = UPnPDeviceMaker::makeDeviceWithDeviceDescription(parser.parse(xmlDoc));
    
		devicePool.updateDevice(device);
		if (deviceListener) {
			deviceListener->onDeviceAdded(device);
		}
    
		UPnPServicePositionMaker maker(device.getUdn());
		vector<UPnPServicePosition> servicePositions = makeServicePositions(maker, device);

		LOOP_VEC(servicePositions, i) {
			UPnPServicePosition & sp = servicePositions[i];
			string scpdurl = sp.getScpdUrl();
			Url url = Url(baseUrl);
			url.setPath(scpdurl);
			UPnPHttpRequestSession session(UPnPHttpRequestType::SCPD_TYPE);
			session.setServicePosition(sp);
			httpClientThreadPool.request(url, "GET", NULL, 0, session);
		}
	}

	void UPnPControlPoint::onScpdInXml(const UPnPServicePosition & servicePosition, string xmlDoc) {
		// TODO: bind scpd to device
		XmlDomParser parser;
		Scpd scpd = UPnPServiceMaker::makeScpdWithXmlDocument("", parser.parse(xmlDoc));
    
		devicePool.bindScpd(servicePosition, scpd);
	}

	void UPnPControlPoint::onDeviceByeBye(string udn) {
		if (!devicePool.hasDevice(udn)) {
			return;
		}
    
		if (deviceListener) {
			deviceListener->onDeviceRemoved(devicePool.getDevice(udn));
		}
		devicePool.removeDevice(udn);
	}

	UPnPDevice UPnPControlPoint::makeDeviceFromXml(XmlNode & xmlNode) {
		return UPnPDeviceMaker::makeDeviceWithDeviceNode(xmlNode);
	}
	UPnPService UPnPControlPoint::makeServiceFromXml(XmlNode & xmlNode) {
		return UPnPServiceMaker::makeServiceWithXmlNode(xmlNode);
	}
	Scpd UPnPControlPoint::makeScpdFromXml(XmlNode & xmlNode) {
		return UPnPServiceMaker::makeScpdFromXmlNode(xmlNode);
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
}
