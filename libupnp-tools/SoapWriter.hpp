#ifndef __SOAP_WRITER_HPP__
#define __SOAP_WRITER_HPP__

#include <string>
#include <vector>

namespace SOAP {
    
    class NameValue {
    private:
        mutable std::string name;
        mutable std::string value;
    public:
        NameValue();
        NameValue(const std::string & name, const std::string & value);
        virtual ~NameValue();
        void setName(const std::string & name);
        void setValue(const std::string & value);
        std::string getName() const;
        std::string getValue() const;
    };
    
    class SoapWriter {
    private:
        std::string urn;
        std::string actionName;
        std::vector<NameValue> arguments;
        
    public:
        SoapWriter();
        virtual ~SoapWriter();
        
        void setSoapAction(const std::string & urn, const std::string & actionName);
        void setArgument(const std::string & name, const std::string & value);
        
        std::string writeArgument(const NameValue & nv) const;
        
        std::string toString() const;
    };
}

#endif
