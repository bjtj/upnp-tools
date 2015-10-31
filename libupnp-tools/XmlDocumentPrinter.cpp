#include "XmlDocumentPrinter.hpp"
#include <iostream>

namespace XML {

	using namespace std;
	
	XmlDocumentPrinter::XmlDocumentPrinter(XmlDocument & doc) : doc(doc) {
	}

	XmlDocumentPrinter::~XmlDocumentPrinter() {
	}

	string XmlDocumentPrinter::printDocument() {
		return printNodeTree(doc.getRootNode());
	}

	string XmlDocumentPrinter::printNodeTree(const XmlNode & node) {
		string ret;
		if (node.isElementNode()) {
			ret.append(printStartTag(node));
			for (size_t i = 0; i < node.getChildren().size(); i++) {
				ret.append(printNodeTree(node[i]));
			}
			ret.append(printEndTag(node));
		} else if (node.isTextNode()) {
			ret.append(printText(node.getData()));
		}
		
		return ret;
	}

	string XmlDocumentPrinter::printStartTag(const XmlNode & node) {
		string tagName = printNamespaceAndName(node.getNamespace(), node.getTagName());
		string attributes;
		for (size_t i = 0; i < node.getAttributes().size(); i++) {
			attributes.append(" " + printAttribute(node.getAttributes()[i]));
		}
		return "<" + tagName + attributes + ">"; 
	}

	string XmlDocumentPrinter::printEndTag(const XmlNode & node) {
		return "</" + printNamespaceAndName(node.getNamespace(), node.getTagName()) + ">";
	}

	string XmlDocumentPrinter::printAttribute(const XmlAttribute & attribute) {
		if (attribute.getName().empty()) {
			return "";
		}
		return printNamespaceAndName(attribute.getNamespace(), attribute.getName()) + "=" + attribute.getValue();
	}

	string XmlDocumentPrinter::printText(const string & text) {
		return text;
	}

    string XmlDocumentPrinter::printNamespaceAndName(const string & ns, const string & name) {
        if (ns.empty()) {
            return name;
        }
        return ns + ":" + name;
    }
	
}
