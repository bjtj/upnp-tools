#ifndef __UPNP_CONTROL_POINT_HPP__
#define __UPNP_CONTROL_POINT_HPP__

#include <liboslayer/PollablePool.hpp>
#include <libhttp-server/AnotherHttpClientThreadPool.hpp>
#include <libhttp-server/HttpClientThreadPool.hpp>
#include <libhttp-server/HttpResponse.hpp>
#include <libhttp-server/HttpRequest.hpp>
#include <string>
#include <vector>
#include <map>
#include "UPnPActionInvoke.hpp"
#include "SSDPServer.hpp"
#include "UPnPDevice.hpp"
#include "UPnPDevicePool.hpp"
#include "UPnPDeviceDetection.hpp"
#include "UPnPServicePosition.hpp"
#include "XmlDocument.hpp"
#include "Timer.hpp"
#include "UniqueIdGenerator.hpp"

namespace UPNP {

	class UPnPControlPoint;
    
	/**
	 * @brief upnp http request types
	 */
    class UPnPHttpRequestType {
	public:
		static const int UNKNOWN;
		static const int DEVICE_DESCRIPTION;
		static const int SCPD;
        static const int ACTION_INVOKE;
    
		static const UPnPHttpRequestType UNKNOWN_TYPE;
		static const UPnPHttpRequestType DEVICE_DESCRIPTION_TYPE;
		static const UPnPHttpRequestType SCPD_TYPE;
        static const UPnPHttpRequestType ACTION_INVOKE_TYPE;
    
	private:
		int type;
    
	public:
		UPnPHttpRequestType();
		UPnPHttpRequestType(int type);
		virtual ~UPnPHttpRequestType();
    
		bool operator==(const int & type);
		bool operator==(const UPnPHttpRequestType & other);
	};

	
	/**
	 * @brief upnp http request session
	 */
	class UPnPHttpRequestSession {
	private:
        ID_TYPE id;
		UPnPDeviceDetection * deviceDetection;
		UPnPHttpRequestType type;
		UPnPServicePosition servicePosition;
        
        UPnPActionRequest actionRequest;
        
	public:
		UPnPHttpRequestSession();
		UPnPHttpRequestSession(const UPnPHttpRequestType & type);
		virtual ~UPnPHttpRequestSession();
        void setId(ID_TYPE id);
        ID_TYPE getId();
		void setServicePosition(const UPnPServicePosition & servicePosition);
		UPnPServicePosition & getServicePosition();
		UPnPHttpRequestType getRequestType();
		void setDeviceDetection(UPnPDeviceDetection * deviceDetection);
        UPnPActionRequest & getUPnPActionRequest();
	};

	
	/**
	 * @brief upnp ssdp notify filter
	 */
	class UPnPControlPointSsdpNotifyFilter {
	private:
		std::vector<std::string> filterTypes;
	public:
		UPnPControlPointSsdpNotifyFilter();
		virtual ~UPnPControlPointSsdpNotifyFilter();
    
		void addFilterType(const std::string & type);
		bool filter(const std::string & type);
	};

	
	/**
	 * @brief device add remove listener
	 */
	class DeviceAddRemoveListener {
	private:
	public:
		DeviceAddRemoveListener() {}
		virtual ~DeviceAddRemoveListener() {}
		virtual void onDeviceAdded(UPnPControlPoint & cp, UPnPDevice & device) = 0;
		virtual void onDeviceRemoved(UPnPControlPoint & cp, UPnPDevice & device) = 0;
	};
    
    /**
     * @brief invoek action response listener
     */
    class InvokeActionResponseListener {
    private:
    public:
        InvokeActionResponseListener() {}
        virtual ~InvokeActionResponseListener() {}
        virtual void onActionResponse(ID_TYPE id, const UPnPActionRequest & actionRequest, const UPnPActionResponse & response) = 0;
    };
	
	/**
	 * @brief upnp ssdp message handler
	 */
	class UPnPSSDPMessageHandler : public SSDP::OnNotifyHandler, public SSDP::OnMsearchHandler, public SSDP::OnHttpResponseHandler {
	private:
		UPnPDeviceDetection * deviceDetection;
	public:
		UPnPSSDPMessageHandler();
		virtual ~UPnPSSDPMessageHandler();
		virtual void onNotify(const HTTP::HttpHeader & header);
        virtual void onMsearch(const HTTP::HttpHeader & header, const OS::InetAddress & remoteAddr);
		virtual void onHttpResponse(const HTTP::HttpHeader & header);
		void setDeviceDetection(UPnPDeviceDetection * deviceDetection);
	};

	/**
	 * @brief upnp control point
	 */
    class UPnPControlPoint : public UPnPDeviceDetection, public HTTP::HttpResponseHandler<UPnPHttpRequestSession>, public UTIL::SelectorPoller, public TimerEvent, public HTTP::OnRequestCompleteListener {
	private:
        UniqueIdGenerator gen;
		UPnPDevicePool devicePool;
		SSDP::SSDPServer ssdp;
		UPnPSSDPMessageHandler ssdpHandler;
		HTTP::HttpClientThreadPool<UPnPHttpRequestSession> httpClientThreadPool;
		UPnPControlPointSsdpNotifyFilter filter;
		DeviceAddRemoveListener * deviceListener;
        InvokeActionResponseListener * invokeActionResponseListener;
        UTIL::PollingThread * pollingThread;
        Timer timer;
        
        HTTP::AnotherHttpClientThreadPool anotherHttpClientThreadPool;

	private:
		UPnPControlPoint(const UPnPControlPoint & other); // do not allow copy
    
	public:
		UPnPControlPoint();
		virtual ~UPnPControlPoint();
        
        virtual void onTimerTriggered();
    
		void start();
		void startAsync();
		void stop();
    
		void sendMsearch(std::string searchType);

        virtual void onHttpResponse(HTTP::HttpClient<UPnPHttpRequestSession> & httpClient, const HTTP::HttpHeader & responseHeader, const std::string & content, UPnPHttpRequestSession userData);
        virtual void onError(HTTP::HttpClient<UPnPHttpRequestSession> & httpClient, UPnPHttpRequestSession userData);

		int getMaxAgeInSecond(const std::string & phrase);
    
        void handleHttpResponse(HTTP::HttpRequest & request, HTTP::HttpResponse & response, const std::string & content, UPnPHttpRequestSession & session);
        
		virtual void onDeviceCacheUpdate(const HTTP::HttpHeader & header);
		virtual void onDeviceHelloWithUrl(const std::string & url, const HTTP::HttpHeader & header);
		virtual void onDeviceDescriptionInXml(std::string baseUrl, std::string xmlDoc);
		virtual void onScpdInXml(const UPnPServicePosition & servicePosition, std::string xmlDoc);
		virtual void onDeviceByeBye(std::string udn);
    
		void setFilter(const UPnPControlPointSsdpNotifyFilter & filter);
		std::vector<UPnPServicePosition> makeServicePositions(UPnPServicePositionMaker & maker, const UPnPDevice & device);
		void setDeviceAddRemoveListener(DeviceAddRemoveListener * deviceListener);
        
        void setInvokeActionResponseListener(InvokeActionResponseListener * invokeActionResponseListener);
        ID_TYPE invokeAction(const UPnPService & service, const std::string & actionName, const UPnPActionParameters & in);
        
        void sendHttpRequest(HTTP::Url & url, const std::string & method, const std::string & content);
        
        virtual void onRequestComplete(HTTP::Url & url, HTTP::HttpResponse & response, const std::string & content, HTTP::UserData * userData);
        virtual void onRequestError(HTTP::Url & url, HTTP::UserData * userData);
	};
}

#endif
