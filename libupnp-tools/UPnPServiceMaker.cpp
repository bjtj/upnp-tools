#include "UPnPServiceMaker.hpp"
#include "XmlNodeFinder.hpp"

namespace UPNP {

	using namespace std;
	using namespace XML;

	UPnPServiceMaker::UPnPServiceMaker() {
	}

	UPnPServiceMaker::~UPnPServiceMaker() {
	}

	UPnPService UPnPServiceMaker::makeWithScpd(const string & serviceType, XmlDocument & doc) {

		UPnPService service;
		service.setServiceType(serviceType);
		XmlNodeFinder finder(doc);
		service["specMajorVersion"] = finder.getContentByTagName("major");
		service["specMinorVersion"] = finder.getContentByTagName("minor");

		vector<XmlNode> serviceStateTable = finder.getAllNodesByTagName("stateVariable");
		for (size_t i = 0; i < serviceStateTable.size(); i++) {
			XmlNode & stateVariableNode = serviceStateTable[i];
			service.getStateVariables().push_back(makeUPnPStateVariable(stateVariableNode));
		}

		vector<XmlNode> actionList = finder.getAllNodesByTagName("action");
		for (size_t i = 0; i < actionList.size(); i++) {
			XmlNode & actionNode = actionList[i];
			service.getActions().push_back(makeUPnPAction(actionNode, service.getStateVariables()));
		}

		return service;
	}

	UPnPStateVariable UPnPServiceMaker::makeUPnPStateVariable(XmlNode & node) {
		UPnPStateVariable stateVariable;
		stateVariable.setName(XmlNodeFinder::getContentByTagName(node, "name"));
		stateVariable.setDataType(XmlNodeFinder::getContentByTagName(node, "dataType"));
		stateVariable["sendEvents"] = node.getAttributeValue("sendEvents");
		stateVariable.setAllowedValueList(XmlNodeFinder::getAllContentsByTagName(node, "allowedValue"));
		return stateVariable;
	}

	UPnPAction UPnPServiceMaker::makeUPnPAction(XmlNode & node, vector<UPnPStateVariable> & serviceStateTable) {
		UPnPAction action;
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

	UPnPStateVariable UPnPServiceMaker::getStateVariable(vector<UPnPStateVariable> & serviceStateTable, string & name) {
		for (size_t i = 0; i < serviceStateTable.size(); i++) {
			if (!serviceStateTable[i].getName().compare(name)) {
				return serviceStateTable[i];
			}
		}
		return UPnPStateVariable();
	}

	UPnPActionArgument UPnPServiceMaker::makeUPnPActionArgument(XmlNode & node, vector<UPnPStateVariable> & serviceStateTable) {
		UPnPActionArgument argument;
		argument.setName(XmlNodeFinder::getContentByTagName(node, "name"));
		argument.setDirection(XmlNodeFinder::getContentByTagName(node, "direction"));
		argument.setStateVariable(getStateVariable(serviceStateTable, XmlNodeFinder::getContentByTagName(node, "relatedStateVariable")));
		return argument;
	}
}