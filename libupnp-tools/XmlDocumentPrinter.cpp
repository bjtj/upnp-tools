#include "XmlDocumentPrinter.hpp"
#include <iostream>

namespace XML {

	using namespace std;
    
    /**
     *
     */
    
    XmlPrinter::XmlPrinter() : formatted(false) {
    }
    
    XmlPrinter::~XmlPrinter() {
    }
    
    void XmlPrinter::setFormatted(bool formatted) {
        this->formatted = formatted;
    }
    
    string XmlPrinter::printDocument(const XmlDocument & doc) {
        return printNodeTree(doc.getRootNode());
    }
    
    string XmlPrinter::printNodeTree(const XmlNode & node) {
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
    
    string XmlPrinter::printStartTag(const XmlNode & node) {
        string tagName = printNamespaceAndName(node.getNamespace(), node.getTagName());
        string attributes;
        for (size_t i = 0; i < node.getAttributes().size(); i++) {
            attributes.append(" " + printAttribute(node.getAttributes()[i]));
        }
        string ret = "<" + tagName + attributes + ">";
        if (formatted) {
            if (node.getChildren().size() > 0 && node.getFirstContent().empty()) {
                ret.append("\n");
            }
            
        }
        return ret;
    }
    
    string XmlPrinter::printEndTag(const XmlNode & node) {
        string ret = "</" + printNamespaceAndName(node.getNamespace(), node.getTagName()) + ">";
        if (formatted) {
            ret.append("\n");
        }
        return ret;
    }
    
    string XmlPrinter::printAttribute(const XmlAttribute & attribute) {
        if (attribute.getName().empty()) {
            return "";
        }
        string ret = printNamespaceAndName(attribute.getNamespace(), attribute.getName()) + "=\"" + attribute.getValue() + "\"";
        return ret;
    }
    
    string XmlPrinter::printText(const string & text) {
        return text;
    }
    
    string XmlPrinter::printNamespaceAndName(const string & ns, const string & name) {
        string ret = name;
        if (!ns.empty()) {
            ret = ns + ":" + name;
        }
        return ret;
    }
    
}
