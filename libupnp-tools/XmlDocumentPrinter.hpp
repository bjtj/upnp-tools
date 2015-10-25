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

		virtual std::string printNodeTree(XmlNode & node);
		virtual std::string printStartTag(XmlNode & node);
		virtual std::string printEndTag(XmlNode & node);
		virtual std::string printAttribute(XmlAttribute & attribute);
		virtual std::string printText(std::string & text);
        virtual std::string printNamespaceAndName(const std::string & ns, const std::string & name);
	};
}

#endif
