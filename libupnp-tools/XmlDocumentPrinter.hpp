#ifndef __XML_DOCUMENT_PRINTER_HPP__
#define __XML_DOCUMENT_PRINTER_HPP__

#include <string>
#include "XmlDocument.hpp"

namespace XML {
	
	class XmlDocumentPrinter {
	private:
		XmlDocument & doc;
		
	public:
		XmlDocumentPrinter(XmlDocument & doc);
		virtual ~XmlDocumentPrinter();

		std::string printDocument();

		virtual std::string printNodeTree(const XmlNode & node);
		virtual std::string printStartTag(const XmlNode & node);
		virtual std::string printEndTag(const XmlNode & node);
		virtual std::string printAttribute(const XmlAttribute & attribute);
		virtual std::string printText(const std::string & text);
        virtual std::string printNamespaceAndName(const std::string & ns, const std::string & name);
	};
}

#endif
