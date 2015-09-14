#ifndef __UPNPSTATEVARIABLE_HPP__
#define __UPNPSTATEVARIABLE_HPP__

#include <string>
#include <vector>

namespace UPNP {
	
	/**
	 * @brief upnp state variable
	 */
	class UPnPStateVariable {
	private:
		std::string name;
		std::string dataType;
		std::string defValue;
		std::vector<std::string> allowedValueList;
		
	public:
		UPnPStateVariable();
		virtual ~UPnPStateVariable();

		void setName(std::string & name);
		std::string getName();
		void setDataType(std::string & dataType);
		std::string getDataType();
		void setDefaultValue(std::string & defValue);
		std::string getDefaultValue();
		std::vector<std::string> & getAllowedValueList();
		void addAllowedValue(std::string & item);
	};
	
}

#endif