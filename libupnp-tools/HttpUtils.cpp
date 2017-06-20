#include "HttpUtils.hpp"
#include <libhttp-server/StringDataSource.hpp>
#include <liboslayer/Logger.hpp>

namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace HTTP;
	using namespace UTIL;

	static AutoRef<Logger> logger = LoggerFactory::getInstance().getObservingLogger(__FILE__);

	unsigned long HttpUtils::connectionTimeout = 5000;
	unsigned long HttpUtils::soTimeout = 5000;

	/**
	 * 
	 */
	
	HttpUtils::DumpResponseHandler::DumpResponseHandler() {
	}
	HttpUtils::DumpResponseHandler::~DumpResponseHandler() {
	}
	AutoRef<DataSink> HttpUtils::DumpResponseHandler::getDataSink() {
		return AutoRef<DataSink>(new StringDataSink);
	}
	void HttpUtils::DumpResponseHandler::onTransferDone(HttpResponse & response, AutoRef<DataSink> sink, AutoRef<UserData> userData) {
		responseHeader = response.header();
		if (!sink.nil()) {
			dump = ((StringDataSink*)&sink)->data();
		}
	}
	void HttpUtils::DumpResponseHandler::onError(OS::Exception & e, AutoRef<UserData> userData) {
		logger->loge("Error/e: " + e.toString());
	}
	HttpResponseHeader & HttpUtils::DumpResponseHandler::getResponseHeader() {
		return responseHeader;
	}
	string & HttpUtils::DumpResponseHandler::getDump() {
		return dump;
	}


	/**
	 * 
	 */
	
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

	void HttpUtils::setRecvTimeout(unsigned long recvTimeout) {
		HttpUtils::soTimeout = recvTimeout;
	}
	
	unsigned long HttpUtils::getRecvTimeout() {
		return HttpUtils::soTimeout;
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

		testHttpErrorCode(url, handler.getResponseHeader().getStatusCode());

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

		testHttpErrorCode(url, handler.getResponseHeader().getStatusCode());

		return handler;
	}

	void HttpUtils::testHttpErrorCode(const Url & url, int code) {
		if (code / 100 != 2) {
			string codeString = Text::toString(code);
			string msg = HttpStatusCodes::getStatusString(code);
			string errorMessage = "http error :: " + codeString + " / " + msg;
			throw Exception(errorMessage + " / url: " + url.toString());
		}
	}
	
}
