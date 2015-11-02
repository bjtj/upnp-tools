#include "SoapWriter.hpp"
#include "macros.hpp"
#include "XmlDocument.hpp"
#include "XmlDomParser.hpp"
#include "XmlDocumentPrinter.hpp"

namespace SOAP {
    
    using namespace std;
    using namespace XML;
    
    NameValue::NameValue() {
    }
    
    NameValue::NameValue(const string & name, const string & value) : name(name), value(value) {
    }
    
    NameValue::~NameValue() {
    }
    
    void NameValue::setName(const string & name) {
        this->name = name;
    }
    void NameValue::setValue(const string & value) {
        this->value = value;
    }
    string NameValue::getName() const {
        return name;
    }
    string NameValue::getValue() const {
        return value;
    }
    
    
    
    SoapWriter::SoapWriter() {
    }
    
    SoapWriter::~SoapWriter() {
    }
    
    void SoapWriter::setSoapAction(const string & urn, const string & actionName) {
        this->urn = urn;
        this->actionName = actionName;
    }
    
    void SoapWriter::setArgument(const string & name, const string & value) {
        LOOP_VEC(arguments, i) {
            NameValue & arg = arguments[i];
            if (!arg.getName().compare(name)) {
                arg.setValue(value);
                return;
            }
        }
        arguments.push_back(NameValue(name, value));
    }
    
    string SoapWriter::writeArgument(const NameValue & nv) const {
        return "<" + nv.getName() + ">" + nv.getValue() + "</" + nv.getName() + ">";
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
            XmlNode argumentNode;
            cursor.enter(argumentNode);
        }
        
        XmlDocument doc;
        doc.setPrologue("<?xml version=\"1.0\"?>");
        doc.setRootNode(cursor.getRootNode());
        XmlDocumentPrinter printer(doc);
        return printer.printDocument();
    }
}