#ifndef __UPNP_RESOURCE_MANAGER_HPP__
#define __UPNP_RESOURCE_MANAGER_HPP__

#include <string>
#include <liboslayer/Properties.hpp>
#include <libhttp-server/Url.hpp>
#include <liboslayer/StringElements.hpp>

namespace UPNP {

	/**
	 * 
	 */
	class UPnPResource {
	private:
		UTIL::LinkedStringMap _meta;
		std::string _content;
	public:
		UPnPResource();
		UPnPResource(const std::string & content);
		virtual ~UPnPResource();
		UTIL::LinkedStringMap & meta();
		std::string & content();
	};

	/**
	 * 
	 */
	class UPnPResourceManager {
	private:
		UTIL::Properties props;
		static UPnPResourceManager _instance;
	private:
		UPnPResourceManager();
		UPnPResourceManager(const UPnPResourceManager & other);
		UPnPResourceManager & operator=(const UPnPResourceManager & other);
	public:
		virtual ~UPnPResourceManager();
		static UPnPResourceManager & instance();
		std::string getResourceContent(const HTTP::Url & url);
		UPnPResource getResource(const HTTP::Url & url);
		UTIL::Properties & properties();
	};
}

#endif
