#include "UPnPActionInvoke.hpp"

namespace UPNP {
    
    using namespace std;
    using namespace UTIL;
    
    /**
     * @brief UPnPActionResult
     */
    
    UPnPActionResult::UPnPActionResult() : success(false), errorCode(0) {
        
    }
    UPnPActionResult::UPnPActionResult(bool success, int errorCode, const string & errorMessage)
    : success(success), errorCode(errorCode), errorMessage(errorMessage) {
        
    }
    UPnPActionResult::~UPnPActionResult() {
    }
    
    bool UPnPActionResult::isSuccess() {
        return success;
    }
    void UPnPActionResult::setSuccess(bool success) {
        this->success = success;
    }
    int UPnPActionResult::getErrorCode() const {
        return errorCode;
    }
    void UPnPActionResult::setErrorCode(int errorCode) {
        this->errorCode = errorCode;
    }
    string UPnPActionResult::getErrorMessage() const {
        return errorMessage;
    }
    void UPnPActionResult::setErrorMessage(const string & errorMessage) {
        this->errorMessage = errorMessage;
    }
    
    
    /**
     * @brief invoek action response listener
     */
    
    UPnPActionParameters::UPnPActionParameters() {
    }
    UPnPActionParameters::~UPnPActionParameters() {
    }
    size_t UPnPActionParameters::size() const {
        return parameters.size();
    }
    
    void UPnPActionParameters::setParameters(const UTIL::LinkedStringMap & parameters) {
        this->parameters = parameters;
    }
	vector<string> UPnPActionParameters::getParameterNames() const {
		vector<string> names;
		for (size_t i = 0; i < parameters.size(); i++) {
			const NameValue & nv = parameters.const_getByIndex(i);
			string name = nv.getName();
			names.push_back(name);
		}
		return names;
	}
	string & UPnPActionParameters::getParameter(const string & name) {
		return parameters[name];
	}
	string UPnPActionParameters::getParameter(const string & name) const {
		return parameters.const_get(name).getValue();
	}
    string & UPnPActionParameters::operator[] (const string & name) {
        return parameters[name];
    }    
    
    /**
     *
     */
    UPnPActionRequest::UPnPActionRequest() {
    }
    UPnPActionRequest::UPnPActionRequest(const UPnPService & service, const string & actionName)
    : service(service), actionName(actionName) {
    }
    
    UPnPActionRequest::UPnPActionRequest(const UPnPActionParameters & other) {
        
    }
    
    UPnPActionRequest::~UPnPActionRequest() {
    }
    
    void UPnPActionRequest::setService(const UPnPService & service) {
        this->service = service;
    }
    UPnPService & UPnPActionRequest::getService() {
        return service;
    }
    void UPnPActionRequest::setActionName(const std::string & actionName) {
        this->actionName = actionName;
    }
    string UPnPActionRequest::getActionName() {
        return actionName;
    }
    
    /**
     *
     */
    
    UPnPActionResponse::UPnPActionResponse() {
    }
    
    UPnPActionResponse::~UPnPActionResponse() {
        
    }
    
    void UPnPActionResponse::setResult(const UPnPActionResult & result) {
        this->result = result;
    }
    UPnPActionResult & UPnPActionResponse::getResult() {
        return result;
    }
}