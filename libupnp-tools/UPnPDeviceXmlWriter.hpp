#ifndef __UPNP_DEVICE_XML_WRITER_HPP__
#define __UPNP_DEVICE_XML_WRITER_HPP__

#include "UPnPDevice.hpp"
#include "UPnPService.hpp"
#include "XmlDocument.hpp"

namespace UPNP {
    
    
    /**
     *
     */
    class UPnPDeviceXmlWriter {
    private:
    public:
        UPnPDeviceXmlWriter();
        virtual ~UPnPDeviceXmlWriter();
        
        static XML::XmlDocument makeDeviceDescriptionXmlDocument(const UPnPDevice & device);
        static XML::XmlNode makeDeviceXmlNode(const UPnPDevice & device);
        static XML::XmlNode makeServiceXmlNode(const UPnPService & service);
        
    private:
        static XML::XmlNode nameValueXml(const std::string & name, const std::string & value);
    };
    
    /**
     *
     */
    class ScpdXmlWriter {
    private:
    public:
        ScpdXmlWriter();
        virtual ~ScpdXmlWriter();
        
        static XML::XmlDocument makeScpdXmlDocument(const Scpd & scpd);
        static XML::XmlNode makeActionXmlNode(const UPnPAction & action);
        static XML::XmlNode makeActionArgumentXmlNode(const UPnPActionArgument & argument);
        static XML::XmlNode makeStateVariableXmlNode(const UPnPStateVariable & stateVariable);
        
    private:
        static XML::XmlNode nameValueXml(const std::string & name, const std::string & value);
    };
    
}

#endif
