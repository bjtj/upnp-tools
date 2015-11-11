#ifndef __SOAP_WRITER_HPP__
#define __SOAP_WRITER_HPP__

#include <string>
#include <vector>
#include <liboslayer/StringElement.hpp>
#include "XmlDocumentPrinter.hpp"

namespace SOAP {
    
    class SoapWriter : public XML::XmlPrinter {
    private:
        std::string urn;
        std::string actionName;
        UTIL::LinkedStringMap arguments;
        std::string prologue;
        
    public:
        SoapWriter();
        virtual ~SoapWriter();
        
        virtual void setSoapAction(const std::string & urn, const std::string & actionName);
        virtual void setArgument(const std::string & name, const std::string & value);
        void setPrologue(const std::string & prologue);
        
        virtual std::string toString() const;
    };
    
    
    class SoapResponseWriter : public SoapWriter {
    private:
    public:
        SoapResponseWriter();
        virtual ~SoapResponseWriter();
        
        virtual void setSoapAction(const std::string & urn, const std::string & actionName);
    };
}

#endif
