#include "UPnPStateVariable.hpp"

namespace UPNP {

	using namespace std;
	
	UPnPStateVariable::UPnPStateVariable() {
	}
	UPnPStateVariable::~UPnPStateVariable() {
	}
	void UPnPStateVariable::setName(const std::string & name) {
		this->name = name;
	}
	string UPnPStateVariable::getName() {
		return name;
	}
	void UPnPStateVariable::setDataType(const std::string & dataType) {
		this->dataType = dataType;
	}
	string UPnPStateVariable::getDataType() {
		return dataType;
	}
	void UPnPStateVariable::setDefaultValue(const std::string & defValue) {
		this->defValue = defValue;
	}
	string UPnPStateVariable::getDefaultValue() {
		return defValue;
	}
	vector<string> & UPnPStateVariable::getAllowedValueList() {
		return allowedValueList;
	}

	void UPnPStateVariable::setAllowedValueList(const vector<string> & list) {
		this->allowedValueList = list;
	}

	void UPnPStateVariable::addAllowedValue(const std::string & item) {
		allowedValueList.push_back(item);
	}
	
	string & UPnPStateVariable::operator[] (const string & name) {
		return properties[name];
	}
}
