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
		UPnPService makeWithScpd(const std::string & serviceType, XML::XmlDocument & doc);

		UPnPStateVariable makeUPnPStateVariable(XML::XmlNode & node);
		UPnPAction makeUPnPAction(XML::XmlNode & node, std::vector<UPnPStateVariable> & serviceStateTable);
		UPnPStateVariable getStateVariable(std::vector<UPnPStateVariable> & serviceStateTable,
										   const std::string & name);
		UPnPActionArgument makeUPnPActionArgument(XML::XmlNode & node,
												  std::vector<UPnPStateVariable> & serviceStateTable);
	};
}

#endif
