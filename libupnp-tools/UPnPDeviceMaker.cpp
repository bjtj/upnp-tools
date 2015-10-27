#include "UPnPDeviceMaker.hpp"
#include "XmlNodeFinder.hpp"
#include "macros.hpp"

namespace UPNP {

	using namespace std;
	using namespace XML;

	UPnPDeviceMaker::UPnPDeviceMaker() {
	}

	UPnPDeviceMaker::~UPnPDeviceMaker() {
	}

	UPnPDevice UPnPDeviceMaker::makeDeviceWithDeviceDescription(XmlDocument & doc) {
		UPnPDevice device;

		XmlNodeFinder finder(doc);
		XmlNode deviceNode = finder.getNodeByTagName("device");

		if (deviceNode.empty()) {
			return device;
		}

		device = makeDeviceWithDeviceNode(deviceNode);

		return device;
	}

	UPnPDevice UPnPDeviceMaker::makeDeviceWithDeviceNode(XML::XmlNode & deviceNode) {
		UPnPDevice device;
		vector<XmlNode> & children = deviceNode.getChildren();
		LOOP_VEC(children, i) {
			XmlNode & node = children[i];
			if (node.isElementNode() && node.getChildrenElementNodeCount() == 0) {
				string name = node.getTagName();
				string value = node.getFirstContent();
				device[name] = value;
			}
		}

		XmlNode deviceListNode = XmlNodeFinder::getNodeByTagName(deviceNode, "deviceList", 1);
		if (!deviceListNode.empty()) {
			vector<XmlNode> deviceNodes = XmlNodeFinder::getAllNodesByTagName(deviceListNode, "device", 1);
			LOOP_VEC(deviceNodes, i) {
				UPnPDevice embeddedDevice = makeDeviceWithDeviceNode(deviceNodes[i]);
				device.addEmbeddedDevice(embeddedDevice);
			}
		}

		return device;
	}
}