#ifndef __HTTP_UTILS_HPP__
#define __HTTP_UTILS_HPP__

#include <string>
#include <liboslayer/os.hpp>
#include <liboslayer/Text.hpp>
#include <libhttp-server/AnotherHttpClient.hpp>
#include <libhttp-server/HttpStatusCodes.hpp>
#include <libhttp-server/Url.hpp>
#include <liboslayer/StringElements.hpp>
#include <libhttp-server/FixedTransfer.hpp>
#include <libhttp-server/StringDataSink.hpp>

namespace upnp {

	class HttpUtils {
	private:
		static unsigned long connectionTimeout;
		static unsigned long soTimeout;
	public:
		/**
		 * 
		 */
		class DumpResponseHandler : public http::OnHttpResponseListener {
		private:
			http::HttpResponseHeader responseHeader;
			std::string dump;
		public:
			DumpResponseHandler();
			virtual ~DumpResponseHandler();
			virtual osl::AutoRef<http::DataSink> getDataSink();
			virtual void onTransferDone(http::HttpResponse & response,
										osl::AutoRef<http::DataSink> sink,
										osl::AutoRef<http::UserData> userData);
			virtual void onError(osl::Exception & e, osl::AutoRef<http::UserData> userData);
			http::HttpResponseHeader & getResponseHeader();
			std::string & getDump();
		};
		
	public:
		HttpUtils();
		virtual ~HttpUtils();
		static void setConnectionTimeout(unsigned long connectionTimeout);
		static unsigned long getConnectionTimeout();
		static void setRecvTimeout(unsigned long readTimeout);
		static unsigned long getRecvTimeout();
		static std::string httpGet(const http::Url & url);
		static std::string httpPost(const http::Url & url,
									const osl::LinkedStringMap & headers,
									const std::string & content);
		static std::string httpPost(const std::string & method,
									const http::Url & url,
									const osl::LinkedStringMap & headers,
									const std::string & content);
		static DumpResponseHandler httpRequest(const http::Url & url,
											   const std::string & method);
		static DumpResponseHandler httpRequest(const http::Url & url,
											   const std::string & method,
											   const osl::LinkedStringMap & headers);
		static DumpResponseHandler httpRequest(const http::Url & url,
											   const std::string & method,
											   const osl::LinkedStringMap & headers,
											   const std::string & content);
		static void testHttpErrorCode(const http::Url & url, HttpUtils::DumpResponseHandler & res);
	};
}

#endif
