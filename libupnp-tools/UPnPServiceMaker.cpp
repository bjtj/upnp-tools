#include "UPnPServiceMaker.hpp"
#include "XmlNodeFinder.hpp"
#include "macros.hpp"

namespace UPNP {

	using namespace std;
	using namespace XML;

	UPnPServiceMaker::UPnPServiceMaker() {
	}

	UPnPServiceMaker::~UPnPServiceMaker() {
	}

	UPnPService UPnPServiceMaker::makeServiceFromXmlNode(const string & baseUrl, const XmlNode & serviceNode) {
		UPnPService service;
        service.setBaseUrl(baseUrl);
		const vector<XmlNode> & nodes = serviceNode.getChildren();
		LOOP_VEC(nodes, i) {
			const XmlNode & node = nodes[i];
			if (node.isElementNode() && node.getChildrenElementNodeCount() == 0) {
				string name = node.getTagName();
				string value = node.getFirstContent();
				service[name] = value;
			}
		}
		return service;
	}

	Scpd UPnPServiceMaker::makeScpdFromXmlDocument(const string & serviceType, const XmlDocument & doc) {

		Scpd scpd;
		XmlNodeFinder finder(doc);
		scpd["specMajorVersion"] = finder.getContentByTagName("major");
		scpd["specMinorVersion"] = finder.getContentByTagName("minor");
        
        return makeScpdFromXmlNode(doc.getRootNode());
	}
    
    Scpd UPnPServiceMaker::makeScpdFromXmlNode(const XmlNode & xmlNode) {
        
        Scpd scpd;
        vector<UPnPStateVariable> stateVariables;
        vector<XmlNode> stateVariableNodes = XmlNodeFinder::getAllNodesByTagName(xmlNode, "stateVariable");
        for (size_t i = 0; i < stateVariableNodes.size(); i++) {
            XmlNode & stateVariableNode = stateVariableNodes[i];
            stateVariables.push_back(makeUPnPStateVariable(stateVariableNode));
        }
        
        vector<UPnPAction> actions;
        vector<XmlNode> actionNodes = XmlNodeFinder::getAllNodesByTagName(xmlNode, "action");
        for (size_t i = 0; i < actionNodes.size(); i++) {
            XmlNode & actionNode = actionNodes[i];
            actions.push_back(makeUPnPAction(actionNode, stateVariables));
        }
        
        scpd.setStateVariables(stateVariables);
        scpd.setActions(actions);
        
        return scpd;
    }

	UPnPStateVariable UPnPServiceMaker::makeUPnPStateVariable(const XmlNode & node) {
		UPnPStateVariable stateVariable;
		stateVariable.setName(XmlNodeFinder::getContentByTagName(node, "name"));
		stateVariable.setDataType(XmlNodeFinder::getContentByTagName(node, "dataType"));
		stateVariable["sendEvents"] = node.getAttributeValue("sendEvents");
		stateVariable.setAllowedValueList(XmlNodeFinder::getAllContentsByTagName(node, "allowedValue"));
		return stateVariable;
	}

	UPnPAction UPnPServiceMaker::makeUPnPAction(const XmlNode & node, const vector<UPnPStateVariable> & serviceStateTable) {
		UPnPAction action;
        
        action.setName(XmlNodeFinder::getContentByTagName(node, "name"));
        
		vector<UPnPActionArgument> arguments;
		vector<XmlNode> argumentNodes = XmlNodeFinder::getAllNodesByTagName(node, "argument");
		for (size_t i = 0; i < argumentNodes.size(); i++) {
			XmlNode & node = argumentNodes[i];
			UPnPActionArgument argument = makeUPnPActionArgument(node, serviceStateTable);
			arguments.push_back(argument);
		}
		action.setArguments(arguments);
		return action;
	}

	UPnPStateVariable UPnPServiceMaker::getStateVariable(const vector<UPnPStateVariable> & serviceStateTable,
														 const string & name) {
		for (size_t i = 0; i < serviceStateTable.size(); i++) {
			if (!serviceStateTable[i].getName().compare(name)) {
				return serviceStateTable[i];
			}
		}
		return UPnPStateVariable();
	}

	UPnPActionArgument UPnPServiceMaker::makeUPnPActionArgument(const XmlNode & node,
																const vector<UPnPStateVariable> & serviceStateTable) {
		UPnPActionArgument argument;
		argument.setName(XmlNodeFinder::getContentByTagName(node, "name"));
		argument.setDirection(XmlNodeFinder::getContentByTagName(node, "direction"));
		argument.setStateVariable(getStateVariable(serviceStateTable,
												   XmlNodeFinder::getContentByTagName(node,
																					  "relatedStateVariable")));
		return argument;
	}
}
