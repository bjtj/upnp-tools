#ifndef __UPNP_SERVICE_MAKER_HPP__
#define __UPNP_SERVICE_MAKER_HPP__

#include "UPnPService.hpp"
#include "UPnPStateVariable.hpp"
#include "UPnPAction.hpp"
#include "UPnPActionArgument.hpp"
#include "XmlDocument.hpp"

namespace UPNP {

	class UPnPServiceMaker {
	private:
	public:
		UPnPServiceMaker();
		virtual ~UPnPServiceMaker();

		static UPnPService makeServiceWithXmlNode(XML::XmlNode & serviceNode);
		static Scpd makeScpdWithXmlDocument(const std::string & serviceType, XML::XmlDocument & doc);
        static Scpd makeScpdFromXmlNode(const XML::XmlNode & xmlNode);

		static UPnPStateVariable makeUPnPStateVariable(XML::XmlNode & node);
		static UPnPAction makeUPnPAction(XML::XmlNode & node, std::vector<UPnPStateVariable> & serviceStateTable);
		static UPnPStateVariable getStateVariable(std::vector<UPnPStateVariable> & serviceStateTable,
										   const std::string & name);
		static UPnPActionArgument makeUPnPActionArgument(XML::XmlNode & node,
												  std::vector<UPnPStateVariable> & serviceStateTable);
	};
}

#endif
