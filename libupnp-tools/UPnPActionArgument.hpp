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

		bool in() const;
		bool out() const;
		std::string getName() const;
		std::string getStateVariableName() const;
        const UPnPStateVariable & getStateVariable() const;
        UPnPStateVariable & getStateVariable();

		void setName(const std::string & name);
		void setStateVariable(const UPnPStateVariable & stateVariable);
		void setDirection(const std::string & direction);
        
        bool hasAllowedValueList();
        std::vector<std::string> getAllowedValueList();
	};
	
}

#endif
