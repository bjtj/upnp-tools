#ifndef __UPNP_CONTROL_POINT_HPP__
#define __UPNP_CONTROL_POINT_HPP__

#include <libhttp-server/HttpServer.hpp>
#include <libhttp-server/HttpClientThreadPool.hpp>

#include <string>
#include <vector>

#include "SSDPServer.hpp"
#include "UPnPDevice.hpp"

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
        UPnPDevice device;
        UPnPDevice * targetDevice;
        UPnPService * targetService;
        
        std::map<std::string, HTTP::Url> scpdTable;
        
    public:
        BuildTarget();
        BuildTarget(UPnPControlPoint * cp);
        virtual ~BuildTarget();
        BuildTarget(const BuildTarget & other);
        
        bool hasTargetdDevice();
        bool hasTagetService();
        UPnPDevice & getDevice();
        void setDevice(const UPnPDevice & device);
        UPnPDevice * getTargetDevice();
        UPnPService * getTargetService();
        void setTargetDevice(UPnPDevice * targetDevice);
        void setTargetService(UPnPService * targetService);
        
        void addScpdUrl(const std::string & serviceType, HTTP::Url & scpdUrl);
        std::map<std::string, HTTP::Url> & getScpdTable();
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
        std::vector<BuildTarget> buildingDevices;
		std::vector<UPnPDevice> devices;
		
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
        BuildTarget registerBuildingDevice(const std::string & deviceDescription);
        void addDevice(UPnPDevice & device);
        void removeDevice(const std::string & udn);
        bool hasDevice(const std::string & udn);
        UPnPDevice makeUPnPDevice(const std::string & deviceDescription);
    };

}

#endif
