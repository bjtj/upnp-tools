#ifndef __XML_DOCUMENT_PRINTER_HPP__
#define __XML_DOCUMENT_PRINTER_HPP__

#include <string>
#include "XmlDocument.hpp"

namespace XML {
    
    /**
     *
     */
    class XmlPrinter {
    private:
		bool showPrologue;
        bool formatted;
    public:
        XmlPrinter();
        virtual ~XmlPrinter();
        
		void setShowPrologue(bool showPrologue);
        void setFormatted(bool formatted);
        
		std::string printPrologue(const XmlDocument & doc);
        std::string printDocument(const XmlDocument & doc);

        virtual std::string printNodeTree(const XmlNode & node);
        virtual std::string printStartTag(const XmlNode & node);
        virtual std::string printEndTag(const XmlNode & node);
        virtual std::string printAttribute(const XmlAttribute & attribute);
        virtual std::string printText(const std::string & text);
        virtual std::string printNamespaceAndName(const std::string & ns, const std::string & name);
    };
    
}

#endif
