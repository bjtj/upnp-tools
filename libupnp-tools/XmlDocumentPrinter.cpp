#include "XmlDocumentPrinter.hpp"
#include "XmlEncoderDecoder.hpp"

namespace XML {

	using namespace std;
    
    /**
     *
     */
    
    XmlPrinter::XmlPrinter() : showPrologue(false), formatted(false) {
    }
    
    XmlPrinter::~XmlPrinter() {
    }
    void XmlPrinter::setShowPrologue(bool showPrologue) {
		this->showPrologue = showPrologue;
	}
    void XmlPrinter::setFormatted(bool formatted) {
        this->formatted = formatted;
    }

	string XmlPrinter::printPrologue(const XmlDocument & doc) const {
		string ret = doc.getPrologue();
		ret.append("\r\n");
		return ret;
	}
    
    string XmlPrinter::printDocument(const XmlDocument & doc) const {
		string ret;
		if (showPrologue) {
			ret.append(printPrologue(doc));
		}
        ret.append(printNodeTree(doc.getRootNode()));
		return ret;
    }
    
    string XmlPrinter::printNodeTree(const XmlNode & node) const {
        string ret;
        if (node.isElementNode()) {
            ret.append(printStartTag(node));
            for (size_t i = 0; i < node.getChildren().size(); i++) {
                ret.append(printNodeTree(node[i]));
            }
            ret.append(printEndTag(node));
        } else if (node.isTextNode()) {
            ret.append(printText(node.getText()));
        }
        
        return ret;
    }
    
    string XmlPrinter::printStartTag(const XmlNode & node) const {
        string tagName = printNamespaceAndName(node.getNamespace(), node.getTagName());
        string attributes;
        for (size_t i = 0; i < node.getAttributes().size(); i++) {
            attributes.append(" " + printAttribute(node.getAttributes()[i]));
        }
        string ret = "<" + tagName + attributes + ">";
        if (formatted) {
            if ((node.getChildren().size() > 1 && node.getFirstContent().empty()) ||
				(node.getChildren().size() == 1 && node.getChildren()[0].isElementNode())) {
                ret.append("\r\n");
            }
            
        }
        return ret;
    }
    
    string XmlPrinter::printEndTag(const XmlNode & node) const {
        string ret = "</" + printNamespaceAndName(node.getNamespace(), node.getTagName()) + ">";
        if (formatted) {
            ret.append("\r\n");
        }
        return ret;
    }
    
    string XmlPrinter::printAttribute(const XmlAttribute & attribute) const {
        if (attribute.getName().empty()) {
            return "";
        }
        string ret = printNamespaceAndName(attribute.getNamespace(), attribute.getName()) + "=\"" + attribute.getValue() + "\"";
        return ret;
    }
    
    string XmlPrinter::printText(const string & text) const {
        return XmlEncoder::encode(text);
    }
    
    string XmlPrinter::printNamespaceAndName(const string & ns, const string & name) const {
        string ret = name;
        if (!ns.empty()) {
            ret = ns + ":" + name;
        }
        return ret;
    }
    
}
