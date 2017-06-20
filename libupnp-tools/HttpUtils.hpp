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

namespace UPNP {

	class HttpUtils {
	private:
		static unsigned long connectionTimeout;
		static unsigned long soTimeout;
	public:
		/**
		 * 
		 */
		class DumpResponseHandler : public HTTP::OnHttpResponseListener {
		private:
			HTTP::HttpResponseHeader responseHeader;
			std::string dump;
		public:
			DumpResponseHandler();
			virtual ~DumpResponseHandler();
			virtual OS::AutoRef<HTTP::DataSink> getDataSink();
			virtual void onTransferDone(HTTP::HttpResponse & response,
										OS::AutoRef<HTTP::DataSink> sink,
										OS::AutoRef<HTTP::UserData> userData);
			virtual void onError(OS::Exception & e, OS::AutoRef<HTTP::UserData> userData);
			HTTP::HttpResponseHeader & getResponseHeader();
			std::string & getDump();
		};
		
	public:
		HttpUtils();
		virtual ~HttpUtils();
		static void setConnectionTimeout(unsigned long connectionTimeout);
		static unsigned long getConnectionTimeout();
		static void setRecvTimeout(unsigned long readTimeout);
		static unsigned long getRecvTimeout();
		static std::string httpGet(const HTTP::Url & url);
		static std::string httpPost(const HTTP::Url & url,
									const UTIL::LinkedStringMap & headers,
									const std::string & content);
		static std::string httpPost(const std::string & method,
									const HTTP::Url & url,
									const UTIL::LinkedStringMap & headers,
									const std::string & content);
		static DumpResponseHandler httpRequest(const HTTP::Url & url,
											   const std::string & method);
		static DumpResponseHandler httpRequest(const HTTP::Url & url,
											   const std::string & method,
											   const UTIL::LinkedStringMap & headers);
		static DumpResponseHandler httpRequest(const HTTP::Url & url,
											   const std::string & method,
											   const UTIL::LinkedStringMap & headers,
											   const std::string & content);
		static void testHttpErrorCode(const HTTP::Url & url, int code);
	};
}

#endif
