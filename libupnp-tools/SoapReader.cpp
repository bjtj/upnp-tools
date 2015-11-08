#include "SoapReader.hpp"
#include "XmlNodeFinder.hpp"
#include "macros.hpp"
#include <vector>

namespace SOAP {
    
    using namespace std;
    using namespace XML;
    using namespace UPNP;
    
    SoapReader::SoapReader() {
    }
    
    SoapReader::~SoapReader() {
    }
    
    XmlNode SoapReader::getActionNode(const XmlNode & xmlNode) {
        return XmlNodeFinder::getNodeByTagName(xmlNode, "Body").getFirstElement();
    }
    
    string SoapReader::getActionNameFromActionNode(const XmlNode & xmlNode) {
        return xmlNode.getTagName();
    }
    
    UPnPActionParameters SoapReader::getActionParametersFromActionNode(const XmlNode & xmlNode) {
        UPnPActionParameters params;
        const vector<XmlNode> & children = xmlNode.getChildren();
        LOOP_VEC(children, i) {
            const XmlNode & node = children[i];
            if (node.isElementNode()) {
                params[node.getTagName()] = node.getFirstContent();
            }
        }
        return params;
    }
}