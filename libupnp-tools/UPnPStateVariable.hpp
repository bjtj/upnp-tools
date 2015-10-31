#ifndef __UPNPSTATEVARIABLE_HPP__
#define __UPNPSTATEVARIABLE_HPP__

#include <string>
#include <vector>
#include <map>

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

		std::map<std::string, std::string> properties;
		
	public:
		UPnPStateVariable();
		virtual ~UPnPStateVariable();

		void setName(const std::string & name);
		std::string getName() const;
		void setDataType(const std::string & dataType);
		std::string getDataType();
		void setDefaultValue(const std::string & defValue);
		std::string getDefaultValue();
		std::vector<std::string> & getAllowedValueList();
		void setAllowedValueList(const std::vector<std::string> & list);
		void addAllowedValue(const std::string & item);

		std::string & operator[] (const std::string & name);
	};
}

#endif
