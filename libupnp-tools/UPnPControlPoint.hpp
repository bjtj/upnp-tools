#ifndef __UPNP_CONTROL_POINT_HPP__
#define __UPNP_CONTROL_POINT_HPP__

#include <libhttp-server/HttpServer.hpp>
#include <libhttp-server/HttpClientThreadPool.hpp>

#include <string>
#include <vector>

#include "SSDPServer.hpp"
#include "UPnPDevice.hpp"

#include "XmlDocument.hpp"

namespace UPNP {
    
    class UPnPControlPoint;

	/**
	 * @brief device add remove listener
	 */
	class OnDeviceAddRemoveListener {
	public:
		OnDeviceAddRemoveListener() {}
		virtual ~OnDeviceAddRemoveListener() {}

		virtual void onDeviceAdd(UPnPDevice & device) = 0;
		virtual void onDeviceRemove(UPnPDevice & device) = 0;
	};

	/**
	 * @brief ssdp handler
	 */
	class ControlPointSSDPHandler : public SSDP::OnNotifyHandler {
    private:
        UPnPControlPoint & cp;
	public:
		ControlPointSSDPHandler(UPnPControlPoint & cp);
		virtual ~ControlPointSSDPHandler();
		virtual void onNotify(HTTP::HttpHeader & header);
	};
    
    /**
     * @brief build target
     */
    class BuildTarget {
    private:
        UPnPControlPoint * cp;
        UPnPDevice * deviceFrame;
        UPnPService * targetService;
		HTTP::Url url;
        
    public:
        BuildTarget();
        BuildTarget(UPnPControlPoint * cp);
        virtual ~BuildTarget();
        
		bool empty();
        bool hasTagetService();
        UPnPDevice * getDeviceFrame();
        void setDeviceFrame(UPnPDevice * deviceFrame);
        UPnPService * getTargetService();
        void setTargetService(UPnPService * targetService);
		HTTP::Url & getUrl();
		void setUrl(HTTP::Url & url);
		UPnPControlPoint * getControlPoint();
    };
    
    /**
     * @brief http response handler
     */
    class ControlPointHttpResponseHandler : public HTTP::HttpResponseHandler<BuildTarget> {
    private:
        UPnPControlPoint & cp;
    public:
        ControlPointHttpResponseHandler(UPnPControlPoint & cp);
        virtual ~ControlPointHttpResponseHandler();
        
        virtual void onResponse(HTTP::HttpClient<BuildTarget> & client,
                                HTTP::HttpHeader & responseHeader,
                                OS::Socket & socket,
                                BuildTarget buildTarget);
    };

	/**
	 * @brief upnp control point
	 */
	class UPnPControlPoint {
	private:
		std::string searchTarget;
		SSDP::SSDPServer ssdpServer;
		ControlPointSSDPHandler ssdpHandler;
		HTTP::HttpServer httpServer;
        std::map<std::string, std::vector<BuildTarget>> buildTargetTable;
		std::map<std::string, UPnPDevice> deviceFrames;
		std::map<std::string, UPnPDevice> devices;
		
		OnDeviceAddRemoveListener * listener;
        
        HTTP::HttpClientThreadPool<BuildTarget> httpClient;
        ControlPointHttpResponseHandler httpResponseHandler;
        
        OS::Semaphore deviceListLock;
		
	public:
		UPnPControlPoint(int port, std::string searchTarget);
		virtual ~UPnPControlPoint();

		virtual void startAsync();
		virtual void stop();
		virtual bool isRunning();

		virtual void sendMsearch();
		virtual std::vector<UPnPDevice> getDeviceList();
		virtual UPnPDevice getDevice(std::string udn);
		virtual void removeAllDevices();

		void setSearchTarget(std::string searchTarget);
		std::string getSearchTaget();

		void setOnDeviceAddRemoveListener(OnDeviceAddRemoveListener * listener);
        
        void ssdpDeviceFound(const std::string & urlString);
        
        void addDevice(UPnPDevice & device);
        void removeDevice(const std::string & udn);
        bool hasDevice(const std::string & udn);
		UPnPDevice & getDevice(const std::string & udn);

		UPnPDevice makeUPnPDeviceFrame(XML::XmlDocument & doc);
		UPnPDevice * registerDeviceFrame(UPnPDevice & deviceFrame);
		void removeDeviceFrame(const std::string & udn);
		std::vector<BuildTarget> makeBuildTargets(UPnPDevice * device, HTTP::Url & url);
		void registerBuildTargets(const std::string & udn, std::vector<BuildTarget> & buildTargets);
		void registerBuildTarget(const std::string & udn, BuildTarget & buildTarget);
		void unregisterBuildTarget(BuildTarget & buildTarget);
		void setScpdToUPnPService(UPnPService & targetService, XML::XmlDocument & doc);

		void handleDeviceDescrition(XML::XmlDocument & doc, HTTP::Url & url);
		void handleScpd(BuildTarget & buildTarget, XML::XmlDocument & doc);
    };
}

#endif
