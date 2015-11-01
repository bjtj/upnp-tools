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
		
	public:
		
		UPnPAction();
		virtual ~UPnPAction();

        void setName(const std::string & name);
		std::string getName();

		std::vector<UPnPActionArgument> & getArguments();
		void setArguments(std::vector<UPnPActionArgument> &);

		bool isEmpty();
	};
	
}

#endif
