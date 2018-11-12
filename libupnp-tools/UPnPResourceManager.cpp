#include "UPnPResourceManager.hpp"
#include "HttpUtils.hpp"
#include <liboslayer/os.hpp>
#include <liboslayer/FileStream.hpp>

namespace upnp {

    using namespace std;
    using namespace http;
    using namespace osl;

    UPnPResource::UPnPResource() {
    }

    UPnPResource::UPnPResource(const string & content) : _content(content) {
    }

    UPnPResource::~UPnPResource() {
    }

    LinkedStringMap & UPnPResource::meta() {
	return _meta;
    }

    string & UPnPResource::content() {
	return _content;
    }
	

    // Properties UPnPResourceManager::props;

    UPnPResourceManager UPnPResourceManager::_instance;
	
    UPnPResourceManager::UPnPResourceManager() { /**/ }

    UPnPResourceManager::~UPnPResourceManager() { /**/ }

    UPnPResourceManager & UPnPResourceManager::instance() {
	return _instance;
    }
	
    string UPnPResourceManager::getResourceContent(const Url & url) {
	return getResource(url).content();
    }

    UPnPResource UPnPResourceManager::getResource(const Url & url) {
	if (url.getScheme() == "http") {
	    UPnPResource res;
	    HttpUtils::DumpResponseHandler handler = HttpUtils::httpRequest(url, "GET");
	    LinkedStringListMap fields = handler.getResponseHeader().getHeaderFields();
	    LinkedStringMap meta;
	    for (size_t i = 0; i < fields.size(); i++) {
		meta[fields[i].name()] = fields[i].obj().first();
	    }
	    res.meta() = meta;
	    res.content() = handler.getDump();
	    return res;
	} else if (url.getScheme() == "file") {
	    UPnPResource res;
	    FileStream fstream(url.getPath(), "rb");
	    res.content() = fstream.readFullAsString();
	    fstream.close();
	    return res;
	} else if (url.getScheme() == "prop") {
	    UPnPResource res;
	    res.content() = props[url.getPath()];
	    return res;
	}

	throw Exception("not supported scheme - " + url.getScheme() + " (" + url.toString() + ")");
    }

    Properties & UPnPResourceManager::properties() {
	return props;
    }
}
