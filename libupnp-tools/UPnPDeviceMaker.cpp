#include "UPnPDeviceMaker.hpp"
#include "UPnPServiceMaker.hpp"
#include "XmlNodeFinder.hpp"
#include "macros.hpp"

namespace UPNP {

	using namespace std;
	using namespace XML;

	UPnPDeviceMaker::UPnPDeviceMaker() {
	}

	UPnPDeviceMaker::~UPnPDeviceMaker() {
	}

	UPnPDevice UPnPDeviceMaker::makeDeviceFromDeviceDescription(const string & baseUrl, const XmlDocument & doc) {
		UPnPDevice device;
        device.setBaseUrl(baseUrl);

		XmlNodeFinder finder(doc);
		XmlNode deviceNode = finder.getNodeByTagName("device");

		if (deviceNode.empty()) {
			return device;
		}

		device = makeDeviceFromDeviceNode(baseUrl, deviceNode);

		return device;
	}

	UPnPDevice UPnPDeviceMaker::makeDeviceFromDeviceNode(const string & baseUrl, const XmlNode & deviceNode) {
		UPnPDevice device;
        device.setBaseUrl(baseUrl);
		const vector<XmlNode> & children = deviceNode.getChildren();
		LOOP_VEC(children, i) {
			const XmlNode & node = children[i];
			if (node.isElementNode() && node.getChildrenElementNodeCount() == 0) {
				string name = node.getTagName();
				string value = node.getFirstContent();
				device[name] = value;
			}
		}

		device.setServices(makeServiceListFromXmlNode(baseUrl, deviceNode));

		XmlNode deviceListNode = XmlNodeFinder::getNodeByTagName(deviceNode, "deviceList", 1);
		if (!deviceListNode.empty()) {
			vector<XmlNode> deviceNodes = XmlNodeFinder::getAllNodesByTagName(deviceListNode, "device", 1);
			LOOP_VEC(deviceNodes, i) {
				UPnPDevice embeddedDevice = makeDeviceFromDeviceNode(baseUrl, deviceNodes[i]);
				device.addEmbeddedDevice(embeddedDevice);
			}
		}

		return device;
	}

	vector<UPnPService> UPnPDeviceMaker::makeServiceListFromXmlNode(const string & baseUrl, const XmlNode & deviceNode) {
		vector<UPnPService> services;
		XmlNode serviceListNode = XmlNodeFinder::getNodeByTagName(deviceNode, "serviceList", 1);
        if (!serviceListNode.empty()) {
            vector<XmlNode> serviceNodes = XmlNodeFinder::getAllNodesByTagName(serviceListNode, "service", 1);
            LOOP_VEC(serviceNodes, i) {
                XmlNode & serviceNode = serviceNodes[i];
                UPnPServiceMaker serviceMaker;
                UPnPService service = serviceMaker.makeServiceFromXmlNode(baseUrl, serviceNode);
                services.push_back(service);
            }
        }
		return services;
	}


}