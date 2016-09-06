#include "UPnPResourceManager.hpp"
#include "HttpUtils.hpp"
#include <liboslayer/os.hpp>
#include <liboslayer/FileStream.hpp>

namespace UPNP {

	using namespace std;
	using namespace HTTP;
	using namespace UTIL;
	using namespace OS;

	Properties UPnPResourceManager::props;
	
	UPnPResourceManager::UPnPResourceManager() {
	}
	
	UPnPResourceManager::~UPnPResourceManager() {
	}
	
	string UPnPResourceManager::getResource(const Url & url) {
		if (url.getScheme() == "http") {
			
			return HttpUtils::httpGet(url);
			
		} else if (url.getScheme() == "file") {
			
			FileStream fstream(url.getPath(), "rb");
			string ret = fstream.readFullAsString();
			fstream.close();
			return ret;
			
		} else if (url.getScheme() == "prop") {
			return props[url.getPath()];
		}
		
		throw Exception("UPnPResourceManager :: unknown scheme - " + url.getScheme());
	}

	Properties & UPnPResourceManager::properties() {
		return props;
	}
}
