#include "UPnPResourceManager.hpp"
#include "HttpUtils.hpp"
#include <liboslayer/os.hpp>
#include <liboslayer/FileStream.hpp>

namespace UPNP {

	using namespace std;
	using namespace HTTP;
	using namespace UTIL;
	using namespace OS;
	
	UPnPResourceManager::UPnPResourceManager() {
	}
	UPnPResourceManager::~UPnPResourceManager() {
	}
	string UPnPResourceManager::getResource(const Url & url) {
		if (url.getScheme() == "http") {
			return HttpUtils::httpGet(url);
		} else if (url.getScheme() == "file") {
			FileStream fstream(url.getPath(), "rb");
			size_t len = 0;
			string ret;
			char buffer[1024] = {0,};
			while ((len = fstream.read(buffer, sizeof(buffer))) > 0) {
				ret.append(buffer, len);
			}
			fstream.close();
			return ret;
		}
		throw Exception("unknown scheme");
	}
}
