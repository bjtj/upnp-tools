#ifndef __UPNP_CONTROL_POINT_HPP__
#define __UPNP_CONTROL_POINT_HPP__

#include <libhttp-server/HttpServer.hpp>
#include <libhttp-server/HttpClientThreadPool.hpp>
#include <string>
#include <vector>
#include "SSDPServer.hpp"
#include "UPnPDevice.hpp"
#include "XmlDocument.hpp"
#include "Timer.hpp"

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
        std::string udn;
        UPnPControlPoint & cp;
        UPnPDevice * device;
        UPnPService * targetService;
		HTTP::Url url;
        
    public:
        BuildTarget(UPnPControlPoint & cp);
        virtual ~BuildTarget();
        
        bool hasTagetService();
        UPnPDevice * getDevice();
        void setDevice(UPnPDevice * device);
        UPnPService * getTargetService();
        void setTargetService(UPnPService * targetService);
		HTTP::Url & getUrl();
		void setUrl(HTTP::Url & url);
		UPnPControlPoint & getControlPoint();
        void setUdn(const std::string & udn);
        std::string getUdn();
    };
    
    /**
     * @brief http response handler
     */
    class ControlPointHttpResponseHandler : public HTTP::HttpResponseHandler<BuildTarget*> {
    private:
        UPnPControlPoint & cp;
    public:
        ControlPointHttpResponseHandler(UPnPControlPoint & cp);
        virtual ~ControlPointHttpResponseHandler();
        
        virtual void onResponse(HTTP::HttpClient<BuildTarget*> & client,
                                HTTP::HttpHeader & responseHeader,
                                OS::Socket & socket,
                                BuildTarget * buildTarget);
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
        std::map<std::string, std::vector<BuildTarget*> > buildTargetTable;
		std::map<std::string, UPnPDevice> devices;
		OnDeviceAddRemoveListener * listener;
        HTTP::HttpClientThreadPool<BuildTarget*> httpClient;
        ControlPointHttpResponseHandler httpResponseHandler;
        OS::Semaphore deviceListLock;
        OS::Semaphore buildTargetLock;
        Timer timer;
		
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
        void ssdpDeviceRemoved(const std::string & udn);
        void addDevice(UPnPDevice & device);
        void removeDevice(const std::string & udn);
        bool hasDevice(const std::string & udn);
		UPnPDevice & getDevice(const std::string & udn);
		UPnPDevice makeUPnPDeviceFrame(XML::XmlDocument & doc);
        BuildTarget * makeBuildTarget(UPnPDevice * device, UPnPService * service, HTTP::Url & url);
		std::vector<BuildTarget*> makeBuildTargets(UPnPDevice * device, HTTP::Url & url);
		void registerBuildTargets(const std::string & udn, std::vector<BuildTarget*> & buildTargets);
		void registerBuildTarget(const std::string & udn, BuildTarget * buildTarget);
		void unregisterBuildTarget(BuildTarget * buildTarget);
		void setScpdToUPnPService(UPnPService * targetService, XML::XmlDocument & doc);
		void handleDeviceDescrition(XML::XmlDocument & doc, HTTP::Url & url);
		void handleScpd(BuildTarget * buildTarget, XML::XmlDocument & doc);
    };
}

#endif
