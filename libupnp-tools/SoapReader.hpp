#ifndef __SOAP_READER_HPP__
#define __SOAP_READER_HPP__

#include <string>
#include "XmlDocument.hpp"
#include "UPnPControlPoint.hpp"

namespace SOAP {
    
    class SoapReader {
    private:
    public:
        SoapReader();
        virtual ~SoapReader();
        
        XML::XmlNode getActionNode(const XML::XmlNode & xmlNode);
        std::string getActionNameFromActionNode(const XML::XmlNode & xmlNode);
        UPNP::ActionParameters getActionParametersFromActionNode(const XML::XmlNode & xmlNode);
        
    };
}

#endif
