#ifndef __UPNPACTION_HPP__
#define __UPNPACTION_HPP__

#include <string>
#include <vector>

#include "UPnPStateVariable.hpp"
#include "UPnPActionArgument.hpp"

namespace UPNP {

	/**
	 * @brief upnp action
	 */
	class UPnPAction {
	private:

		std::string name;
		std::vector<UPnPActionArgument> arguments;
		std::vector<UPnPStateVariable> stateVariables;
		
	public:
		
		UPnPAction();
		virtual ~UPnPAction();

		std::string getName();

		std::vector<UPnPActionArgument> & getArguments();
		std::vector<UPnPStateVariable> & getStateVariables();

		bool isEmpty();
	};
	
}

#endif
