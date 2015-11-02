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

        static UPnPService makeServiceFromXmlNode(const std::string & baseUrl, const XML::XmlNode & serviceNode);
		static Scpd makeScpdFromXmlDocument(const std::string & serviceType, const XML::XmlDocument & doc);
        static Scpd makeScpdFromXmlNode(const XML::XmlNode & xmlNode);

		static UPnPStateVariable makeUPnPStateVariable(const XML::XmlNode & node);
		static UPnPAction makeUPnPAction(const XML::XmlNode & node, const std::vector<UPnPStateVariable> & serviceStateTable);
		static UPnPStateVariable getStateVariable(const std::vector<UPnPStateVariable> & serviceStateTable,
										   const std::string & name);
		static UPnPActionArgument makeUPnPActionArgument(const XML::XmlNode & node,
												  const std::vector<UPnPStateVariable> & serviceStateTable);
	};
}

#endif
