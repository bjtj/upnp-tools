#include "HttpUtils.hpp"

namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace HTTP;
	using namespace UTIL;

	unsigned long HttpUtils::connectionTimeout = 5000;
	
	HttpUtils::HttpUtils() {
	}
	
	HttpUtils::~HttpUtils() {
	}

	void HttpUtils::setConnectionTimeout(unsigned long connectionTimeout) {
		HttpUtils::connectionTimeout = connectionTimeout;
	}
	
	unsigned long HttpUtils::getConnectionTimeout() {
		return HttpUtils::connectionTimeout;
	}

	HttpUtils::DumpResponseHandler HttpUtils::dumpHttpRequest(const Url & url, const string & method, const LinkedStringMap & headers) {
		AnotherHttpClient client;
		client.setConnectionTimeout(connectionTimeout);
    
		HttpUtils::DumpResponseHandler handler;
		client.setOnResponseListener(&handler);
    
		client.setFollowRedirect(true);
		client.setUrl(url);
		client.setRequest(method, headers, NULL);
		client.execute();

		return handler;
	}

	string HttpUtils::httpGet(const Url & url) {
		AnotherHttpClient client;
		client.setConnectionTimeout(connectionTimeout);
    
		DumpResponseHandler handler;
		client.setOnResponseListener(&handler);
    
		client.setFollowRedirect(true);
		client.setUrl(url);
		client.setRequest("GET", LinkedStringMap(), NULL);
		client.execute();

		testHttpError(handler.getResponseHeader().getStatusCode());

		return handler.getDump();
	}

	string HttpUtils::httpPost(const Url & url, const LinkedStringMap & headers, const string & content) {
		AnotherHttpClient client;
		client.setConnectionTimeout(connectionTimeout);
		
		DumpResponseHandler handler;
		client.setOnResponseListener(&handler);
		client.setFollowRedirect(true);
		client.setUrl(url);
		client.setRequest("POST", headers, new FixedTransfer(content));
		client.execute();

		testHttpError(handler.getResponseHeader().getStatusCode());
			
		return handler.getDump();
	}

	void HttpUtils::testHttpError(int code) {
		if (code / 100 != 2) {
			string codeString = Text::toString(code);
			string msg = HttpStatusCodes::getMessage(code);
			throw Exception("http error - " + codeString + " / " + msg, -1, 0);
		}
	}
	
}
