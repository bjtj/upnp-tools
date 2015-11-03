#include "SoapWriter.hpp"
#include "macros.hpp"
#include "XmlDocument.hpp"
#include "XmlDomParser.hpp"
#include "XmlDocumentPrinter.hpp"

namespace SOAP {
    
    using namespace std;
    using namespace XML;
	using namespace UTIL;
    
    
    SoapWriter::SoapWriter() {
    }
    
    SoapWriter::~SoapWriter() {
    }
    
    void SoapWriter::setSoapAction(const string & urn, const string & actionName) {
        this->urn = urn;
        this->actionName = actionName;
    }
    
    void SoapWriter::setArgument(const string & name, const string & value) {
		arguments[name] = value;
    }
    
    string SoapWriter::writeArgument(const string & name, const string & value) const {
        return "<" + name + ">" + value + "</" + name + ">";
    }
    
    string SoapWriter::toString() const {
        
        XmlParseCursor cursor;
        
        XmlNode envelopeNode;
        envelopeNode.setNamespace("s");
        envelopeNode.setTagName("Envelope");
        envelopeNode.setAttribute("xmlns:s", "http://schemas.xmlsoap.org/soap/envelope/");
        envelopeNode.setAttribute("s:encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");
        cursor.enter(envelopeNode);
        
        XmlNode bodyNode;
        bodyNode.setNamespace("s");
        bodyNode.setTagName("Body");
        cursor.enter(bodyNode);
        
        XmlNode actionNode;
        actionNode.setNamespace("u");
        actionNode.setTagName(actionName);
        actionNode.setAttribute("xmlns:u", urn);
        cursor.enter(actionNode);
        
        LOOP_VEC(arguments, i) {
            NameValue argument = arguments[i];
            XmlNode argumentNameNode;
			XmlNode argumentValueNode;
			argumentNameNode.setTagName(argument.getName());
			argumentValueNode.setData(argument.getValue());
            cursor.enter(argumentNameNode);
			cursor.append(argumentValueNode);
			cursor.leave();
        }

		cursor.leave();
		cursor.leave();
		cursor.leave();
        
        XmlDocument doc;
        doc.setPrologue("<?xml version=\"1.0\"?>");
        doc.setRootNode(cursor.getRootNode());
        XmlDocumentPrinter printer(doc);
        return printer.printDocument();
    }
}