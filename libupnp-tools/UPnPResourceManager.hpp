#ifndef __UPNP_RESOURCE_MANAGER_HPP__
#define __UPNP_RESOURCE_MANAGER_HPP__

#include <string>
#include <liboslayer/Properties.hpp>
#include <libhttp-server/Url.hpp>
#include <liboslayer/StringElements.hpp>

namespace upnp {

    /**
     * 
     */
    class UPnPResource {
    private:
	osl::LinkedStringMap _meta;
	std::string _content;
    public:
	UPnPResource();
	UPnPResource(const std::string & content);
	virtual ~UPnPResource();
	osl::LinkedStringMap & meta();
	std::string & content();
    };

    /**
     * 
     */
    class UPnPResourceManager {
    private:
	osl::Properties props;
	static UPnPResourceManager _instance;
    private:
	UPnPResourceManager();
	UPnPResourceManager(const UPnPResourceManager & other);
	UPnPResourceManager & operator=(const UPnPResourceManager & other);
    public:
	virtual ~UPnPResourceManager();
	static UPnPResourceManager & instance();
	std::string getResourceContent(const http::Url & url);
	UPnPResource getResource(const http::Url & url);
	osl::Properties & properties();
    };
}

#endif
