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
        
		std::string printPrologue(const XmlDocument & doc) const;
        std::string printDocument(const XmlDocument & doc) const;

        virtual std::string printNodeTree(const XmlNode & node) const;
        virtual std::string printStartTag(const XmlNode & node) const;
        virtual std::string printEndTag(const XmlNode & node) const;
        virtual std::string printAttribute(const XmlAttribute & attribute) const;
        virtual std::string printText(const std::string & text) const;
        virtual std::string printNamespaceAndName(const std::string & ns, const std::string & name) const;
    };
    
}

#endif
