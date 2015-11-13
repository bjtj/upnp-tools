#include "UPnPDeviceMaker.hpp"
#include "UPnPDeviceIterator.hpp"
#include "UPnPServiceMaker.hpp"
#include "XmlNodeFinder.hpp"
#include "macros.hpp"

#include "XmlDomParser.hpp"
#include <liboslayer/os.hpp>
#include <liboslayer/Text.hpp>
#include <liboslayer/FileReaderWriter.hpp>

namespace UPNP {

	using namespace std;
	using namespace XML;
	using namespace OS;
	using namespace UTIL;

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
	

	/**
	 * @brief UPnPDeviceMakerFromFile
	 */

	UPnPDeviceMakerFromFile::UPnPDeviceMakerFromFile(LinkedStringMap & replacements) : replacements(replacements) {
	}
	UPnPDeviceMakerFromFile::~UPnPDeviceMakerFromFile() {
	}

	UPnPDevice UPnPDeviceMakerFromFile::makeDeviceFromXmlFile(const string & deviceDescriptionPath, StringMap & scpdPathTable) {
		
		XmlDocument doc = parseXmlFile(deviceDescriptionPath);

		UPnPDevice device = UPnPDeviceMaker::makeDeviceFromDeviceDescription("file:://" + deviceDescriptionPath, doc);
		
		class ScpdBinderIteratorHandler : public IteratorHandler<UPnPService> {
		private:
			UPnPDeviceMakerFromFile & maker;
			StringMap & scpdPathTable;

		public:
			ScpdBinderIteratorHandler(UPnPDeviceMakerFromFile & maker, StringMap & scpdPathTable)
				: maker(maker), scpdPathTable(scpdPathTable) {}
			virtual ~ScpdBinderIteratorHandler() {}
			virtual void onItem(UPnPService & service) {
				string serviceType = service.getServiceType();
				string scpdPath = scpdPathTable[serviceType];
				Scpd scpd = maker.makeScpdFromXmlFile(serviceType, scpdPath);
				service.setScpd(scpd);
			}
		};

		ScpdBinderIteratorHandler handler(*this, scpdPathTable);
		UPnPDeviceIterator::iterateServicesRecursive(device, handler);

		return device;
	}

	Scpd UPnPDeviceMakerFromFile::makeScpdFromXmlFile(const string & serviceType, const string & scpdPath) {

		XmlDocument doc = parseXmlFile(scpdPath);
		return UPnPServiceMaker::makeScpdFromXmlDocument(serviceType, doc);
	}

	XmlDocument UPnPDeviceMakerFromFile::parseXmlFile(const std::string & filePath) {
		XmlDomParser parser;
		return parser.parse(dumpFile(filePath));
	}

	string UPnPDeviceMakerFromFile::dumpFile(const string & filePath) {
		File file(filePath);
		if (!file.exists() || !file.isFile()) {
			throw Exception("invalid file/path: " + filePath, -1, 0);
		}

		FileReader reader(file);
		return replaceAll(reader.dumpAsString());
	}

	string UPnPDeviceMakerFromFile::replaceAll(const string & text) {
		string ret = text;
		for (size_t i = 0; i < replacements.size(); i++) {
			NameValue & nv = replacements[i];
			ret = Text::replaceAll(ret, nv.getName(), nv.getValue());
		}
		return ret;
	}
}