#ifndef __UPNP_ACTION_INVOKE_HPP__
#define __UPNP_ACTION_INVOKE_HPP__

#include <liboslayer/StringElement.hpp>
#include "UPnPService.hpp"
#include <string>

namespace UPNP {
    
    /**
     * @brief action parameters
     */
    class UPnPActionParameters {
    private:
        UTIL::LinkedStringMap params;
        
    public:
        UPnPActionParameters();
        virtual ~UPnPActionParameters();
        size_t size() const;
        const std::string & operator[] (const std::string & name) const;
        std::string & operator[] (const std::string & name);
        const UTIL::NameValue & operator[] (size_t index) const;
        UTIL::NameValue & operator[] (size_t index);
    };
    
    
    /**
     * @brief
     */
    class UPnPActionRequest {
    public:
        UPnPService service;
        std::string actionName;
        UPnPActionParameters inParameters;
        
    public:
        UPnPActionRequest();
        virtual ~UPnPActionRequest();
    };
    
    /**
     * @brief
     */
    class UPnPActionResponse {
    public:
        UPnPActionParameters outParameters;
        
    public:
        UPnPActionResponse();
        virtual ~UPnPActionResponse();
    };
}

#endif
