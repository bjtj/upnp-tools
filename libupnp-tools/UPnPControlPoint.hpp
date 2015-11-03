#ifndef __UPNP_CONTROL_POINT_HPP__
#define __UPNP_CONTROL_POINT_HPP__

#include <libhttp-server/HttpServer.hpp>
#include <libhttp-server/HttpClientThreadPool.hpp>
#include <string>
#include <vector>
#include <map>
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
     * @brief action parameters
     */
    class ActionParameters {
    private:
		UTIL::LinkedStringMap params;
    public:
        ActionParameters();
        virtual ~ActionParameters();
		size_t size() const;
        const std::string & operator[] (const std::string & name) const;
		std::string & operator[] (const std::string & name);
		const UTIL::NameValue & operator[] (size_t index) const;
		UTIL::NameValue & operator[] (size_t index);
    };
    
    /**
     * @brief invoek action response listener
     */
    class InvokeActionResponseListener {
    private:
    public:
        InvokeActionResponseListener() {}
        virtual ~InvokeActionResponseListener() {}
        virtual void onActionResponse(const UPnPService & service, const std::string & actionName, const ActionParameters & in, const ActionParameters & out) = 0;
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
		virtual void onNotify(HTTP::HttpHeader & header);
		virtual void onMsearch(HTTP::HttpHeader & header);
		virtual void onHttpResponse(HTTP::HttpHeader & header);
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
    
	public:
		UPnPControlPoint();
		virtual ~UPnPControlPoint();
    
		void start();
		void startAsync();
		void poll(unsigned long timeout);
		void stop();
    
		void sendMsearch(std::string searchType);
    
		virtual void onDeviceHelloWithUrl(const std::string & url, const HTTP::HttpHeader & header);
		virtual void onDeviceDescriptionInXml(std::string baseUrl, std::string xmlDoc);
		virtual void onScpdInXml(const UPnPServicePosition & servicePosition, std::string xmlDoc);
		virtual void onDeviceByeBye(std::string udn);
    
		virtual void onResponse(HTTP::HttpClient<UPnPHttpRequestSession> & httpClient, HTTP::HttpHeader & responseHeader, OS::Socket & socket, UPnPHttpRequestSession userData);
    
		void setFilter(const UPnPControlPointSsdpNotifyFilter & filter);
		std::vector<UPnPServicePosition> makeServicePositions(UPnPServicePositionMaker & maker, const UPnPDevice & device);
		void setDeviceAddRemoveListener(DeviceAddRemoveListener * deviceListener);
        
        void setInvokeActionResponseListener(InvokeActionResponseListener * invokeActionResponseListener);
        ID_TYPE invokeAction(const UPnPService & service, const std::string & actionName, const ActionParameters & in);
	};
}

#endif
