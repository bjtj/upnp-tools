#ifndef __SOAP_WRITER_HPP__
#define __SOAP_WRITER_HPP__

#include <string>
#include <vector>
#include <liboslayer/StringElement.hpp>

namespace SOAP {
    
    class SoapWriter {
    private:
        std::string urn;
        std::string actionName;
        UTIL::LinkedStringMap arguments;
        
    public:
        SoapWriter();
        virtual ~SoapWriter();
        
        void setSoapAction(const std::string & urn, const std::string & actionName);
        void setArgument(const std::string & name, const std::string & value);
        
        std::string writeArgument(const std::string & name, const std::string & value) const;
        
        std::string toString() const;
    };
}

#endif
