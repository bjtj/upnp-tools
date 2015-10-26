#include "UPnPStateVariable.hpp"

namespace UPNP {

	using namespace std;
	
	UPnPStateVariable::UPnPStateVariable() {
	}
	UPnPStateVariable::~UPnPStateVariable() {
	}
	void UPnPStateVariable::setName(std::string & name) {
		this->name = name;
	}
	string UPnPStateVariable::getName() {
		return name;
	}
	void UPnPStateVariable::setDataType(std::string & dataType) {
		this->dataType = dataType;
	}
	string UPnPStateVariable::getDataType() {
		return dataType;
	}
	void UPnPStateVariable::setDefaultValue(std::string & defValue) {
		this->defValue = defValue;
	}
	string UPnPStateVariable::getDefaultValue() {
		return defValue;
	}
	vector<string> & UPnPStateVariable::getAllowedValueList() {
		return allowedValueList;
	}

	void UPnPStateVariable::setAllowedValueList(vector<string> & list) {
		this->allowedValueList = list;
	}

	void UPnPStateVariable::addAllowedValue(std::string & item) {
		allowedValueList.push_back(item);
	}
	
	string & UPnPStateVariable::operator[] (const string & name) {
		return properties[name];
	}
}
