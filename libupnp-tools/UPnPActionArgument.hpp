#ifndef __UPNPACTIONARGUMENT_HPP__
#define __UPNPACTIONARGUMENT_HPP__

#include <string>

#include "UPnPStateVariable.hpp"
#include "UPnPActionArgumentDirection.hpp"

namespace UPNP {

	/**
	 * @brief upnp action argument
	 */
	class UPnPActionArgument {
	private:
		std::string name;
		StateVariable stateVariable;
		argument_direction_e direction;
		
	public:
		UPnPActionArgument();
		virtual ~UPnPActionArgument();

		bool inArgument();
		bool outArgument();
		std::string getName();
		std::string getStateVariableName();
	};
	
}

#endif
