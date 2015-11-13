#ifndef __UPNP_DEVICE_MAKER_HPP__
#define __UPNP_DEVICE_MAKER_HPP__

#include "UPnPDevice.hpp"
#include "XmlDocument.hpp"
#include <liboslayer/StringElement.hpp>

namespace UPNP {

	class UPnPDeviceMaker {
	private:
	public:
		UPnPDeviceMaker();
		virtual ~UPnPDeviceMaker();

        static UPnPDevice makeDeviceFromDeviceDescription(const std::string & baseUrl, const XML::XmlDocument & doc);
		static UPnPDevice makeDeviceFromDeviceNode(const std::string & baseUrl, const XML::XmlNode & deviceNode);
		static std::vector<UPnPService> makeServiceListFromXmlNode(const std::string & baseUrl, const XML::XmlNode & deviceNode);
	};

	/**
	 * @brief UPnPDeviceMakerFromFile
	 */

	class UPnPDeviceMakerFromFile {
	private:
		UTIL::LinkedStringMap replacements;

	public:
		UPnPDeviceMakerFromFile(UTIL::LinkedStringMap & replacements);
		virtual ~UPnPDeviceMakerFromFile();

		UPnPDevice makeDeviceFromXmlFile(const std::string & deviceDescriptionPath, UTIL::StringMap & scpdPathTable);
		Scpd makeScpdFromXmlFile(const std::string & serviceType, const std::string & scpdPath);

	private:
		XML::XmlDocument parseXmlFile(const std::string & filePath);
		std::string dumpFile(const std::string & filePath);
		std::string replaceAll(const std::string & text);
	};
}

#endif