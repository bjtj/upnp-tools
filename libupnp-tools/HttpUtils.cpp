#include "HttpUtils.hpp"
#include <libhttp-server/StringDataSource.hpp>

namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace HTTP;
	using namespace UTIL;

	unsigned long HttpUtils::connectionTimeout = 5000;
	unsigned long HttpUtils::soTimeout = 5000;
	
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

	string HttpUtils::httpGet(const Url & url) {
		return httpRequest(url, "GET", LinkedStringMap()).getDump();
	}

	string HttpUtils::httpPost(const Url & url, const LinkedStringMap & headers, const string & content) {
		return httpPost("POST", url, headers, content);
	}

	string HttpUtils::httpPost(const string & method, const Url & url, const LinkedStringMap & headers, const string & content) {
		return httpRequest(url, method, headers, content).getDump();
	}

	HttpUtils::DumpResponseHandler HttpUtils::httpRequest(const Url & url, const string & method) {
		return httpRequest(url, method, LinkedStringMap());
	}

	HttpUtils::DumpResponseHandler HttpUtils::httpRequest(const Url & url, const string & method, const LinkedStringMap & headers) {
		AnotherHttpClient client;
		client.setConnectionTimeout(connectionTimeout);
		client.setRecvTimeout(soTimeout);
    
		HttpUtils::DumpResponseHandler handler;
		client.setOnHttpResponseListener(&handler);
    
		client.setFollowRedirect(true);
		client.setUrl(url);
		client.setRequest(method, headers);
		client.execute();

		testHttpErrorCode(handler.getResponseHeader().getStatusCode());

		return handler;
	}

	HttpUtils::DumpResponseHandler HttpUtils::httpRequest(const Url & url, const string & method, const LinkedStringMap & headers, const string & content) {
		AnotherHttpClient client;
		client.setConnectionTimeout(connectionTimeout);
		client.setRecvTimeout(soTimeout);
    
		HttpUtils::DumpResponseHandler handler;
		client.setOnHttpResponseListener(&handler);
    
		client.setFollowRedirect(true);
		client.setUrl(url);
		AutoRef<DataSource> source(new StringDataSource(content));
		AutoRef<DataTransfer> transfer(new FixedTransfer(source, content.size()));
		client.setRequestWithFixedTransfer(method, headers, transfer, content.size());
		client.execute();

		testHttpErrorCode(handler.getResponseHeader().getStatusCode());

		return handler;
	}

	void HttpUtils::testHttpErrorCode(int code) {
		if (code / 100 != 2) {
			string codeString = Text::toString(code);
			string msg = HttpStatusCodes::getStatusString(code);
			string errorMessage = "http error - " + codeString + " / " + msg;
			throw Exception(errorMessage);
		}
	}
	
}
