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
	
	UPnPResourceManager::UPnPResourceManager() { /**/ }
	
	UPnPResourceManager::~UPnPResourceManager() { /**/ }
	
	string UPnPResourceManager::getResource(const Url & url) {
		LinkedStringMap m;
		return getResourceWithMeta(url, m);
	}

	string UPnPResourceManager::getResourceWithMeta(const Url & url, LinkedStringMap & out_meta) {
		if (url.getScheme() == "http") {
			HttpUtils::DumpResponseHandler handler = HttpUtils::httpRequest(url, "GET");
			LinkedStringListMap fields = handler.getResponseHeader().getHeaderFields();
			for (size_t i = 0; i < fields.size(); i++) {
				out_meta[fields[i].name()] = fields[i].first();
			}
			return handler.getDump();
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
