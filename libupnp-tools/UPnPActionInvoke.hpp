#ifndef __UPNP_ACTION_INVOKE_HPP__
#define __UPNP_ACTION_INVOKE_HPP__

#include <liboslayer/StringElement.hpp>
#include "UPnPService.hpp"
#include <string>

namespace UPNP {
    
    class UPnPActionResult {
    private:
        bool success;
        int errorCode;
        std::string errorMessage;
        
    public:
        UPnPActionResult();
        UPnPActionResult(bool success, int errorCode, const std::string & errorMessage);
        virtual ~UPnPActionResult();
        
        bool isSuccess();
        void setSuccess(bool success);
        int getErrorCode() const;
        void setErrorCode(int errorCode);
        std::string getErrorMessage() const;
        void setErrorMessage(const std::string & errorMessage);
    };
    
    /**
     * @brief action parameters
     */
    class UPnPActionParameters {
    private:
        UTIL::LinkedStringMap parameters;
        
    public:
        UPnPActionParameters();
        virtual ~UPnPActionParameters();
        size_t size() const;

        void setParameters(const UTIL::LinkedStringMap & parameters);
		void setParameters(const UPnPActionParameters & parameters);
		std::vector<std::string> getParameterNames() const;
		std::string & getParameter(const std::string & name);
		std::string getParameter(const std::string & name) const;
        
        std::string & operator[] (const std::string & name);
    };
    
    
    /**
     * @brief
     */
    class UPnPActionRequest : public UPnPActionParameters {
    private:
        UPnPService service;
        std::string actionName;
        
    public:
        UPnPActionRequest();
        UPnPActionRequest(const UPnPService & service, const std::string & actionName);
        virtual ~UPnPActionRequest();
        
        void setService(const UPnPService & service);
        UPnPService & getService();
		const UPnPService & getService() const;
        void setActionName(const std::string & actionName);
        std::string getActionName() const;
    };
    
    /**
     * @brief
     */
    class UPnPActionResponse : public UPnPActionParameters {
    private:
        UPnPActionResult result;
        
    public:
        UPnPActionResponse();
        virtual ~UPnPActionResponse();
        
        void setResult(const UPnPActionResult & result);
        UPnPActionResult & getResult();
    };
}

#endif
