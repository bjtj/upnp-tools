#ifndef __UPNP_CONTROL_POINT_HPP__
#define __UPNP_CONTROL_POINT_HPP__

#include <libhttp-server/HttpServer.hpp>
#include <libhttp-server/HttpClientThreadPool.hpp>
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
        virtual void onActionResponse(ID_TYPE id, const UPnPActionRequest & actionRequest, const UPnPActionParameters & out) = 0;
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
	class UPnPControlPoint : public UPnPDeviceDetection, public HTTP::HttpResponseHandler<UPnPHttpRequestSession> {
	private:
        UniqueIdGenerator gen;
		UPnPDevicePool devicePool;
		SSDP::SSDPServer ssdp;
		UPnPSSDPMessageHandler ssdpHandler;
		HTTP::HttpClientThreadPool<UPnPHttpRequestSession> httpClientThreadPool;
		UPnPControlPointSsdpNotifyFilter filter;
		DeviceAddRemoveListener * deviceListener;
        InvokeActionResponseListener * invokeActionResponseListener;

	private:
		UPnPControlPoint(const UPnPControlPoint & other); // do not allow copy
    
	public:
		UPnPControlPoint();
		virtual ~UPnPControlPoint();
    
		void start();
		void startAsync();
		void poll(unsigned long timeout);
		void stop();
    
		void sendMsearch(std::string searchType);

        virtual void onHttpResponse(HTTP::HttpClient<UPnPHttpRequestSession> & httpClient, const HTTP::HttpHeader & responseHeader, const std::string & content, UPnPHttpRequestSession userData);
        virtual void onError(HTTP::HttpClient<UPnPHttpRequestSession> & httpClient, UPnPHttpRequestSession userData);

		int getMaxAgeInSecond(const std::string & phrase);
    
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
	};
}

#endif
