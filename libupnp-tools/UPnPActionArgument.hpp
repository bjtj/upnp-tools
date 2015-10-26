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
		UPnPStateVariable stateVariable;
		std::string direction;
		
	public:
		UPnPActionArgument();
		virtual ~UPnPActionArgument();

		bool inArgument();
		bool outArgument();
		std::string getName();
		std::string getStateVariableName();

		void setName(const std::string & name);
		void setStateVariable(UPnPStateVariable & stateVariable);
		void setDirection(const std::string & direction);
	};
	
}

#endif
