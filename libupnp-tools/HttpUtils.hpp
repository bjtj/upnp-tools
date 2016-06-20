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

		class DumpResponseHandler : public HTTP::OnResponseListener {
		private:
			HTTP::HttpResponseHeader responseHeader;
			std::string dump;
		public:
			DumpResponseHandler() {
			}
			virtual ~DumpResponseHandler() {
			}
			virtual UTIL::AutoRef<HTTP::DataSink> getDataSink() {
				return UTIL::AutoRef<HTTP::DataSink>(new HTTP::StringDataSink);
			}
			virtual void onTransferDone(HTTP::HttpResponse & response, UTIL::AutoRef<HTTP::DataSink> sink, UTIL::AutoRef<HTTP::UserData> userData) {
				responseHeader = response.getHeader();
				if (!sink.nil()) {
					dump = ((HTTP::StringDataSink*)&sink)->data();
				}
			}
			virtual void onError(OS::Exception & e, UTIL::AutoRef<HTTP::UserData> userData) {
				printf("Error/e: %s\n", e.getMessage().c_str());
			}
			HTTP::HttpResponseHeader & getResponseHeader() {
				return responseHeader;
			}
			std::string & getDump() {
				return dump;
			}
		};
		
	public:
		HttpUtils();
		virtual ~HttpUtils();

		static void setConnectionTimeout(unsigned long connectionTimeout);
		static unsigned long getConnectionTimeout();

		static DumpResponseHandler dumpHttpRequest(const HTTP::Url & url, const std::string & method, const UTIL::LinkedStringMap & headers);
		static std::string httpGet(const HTTP::Url & url);
		static std::string httpPost(const HTTP::Url & url, const UTIL::LinkedStringMap & headers, const std::string & content);
		static std::string httpPost(const std::string & method, const HTTP::Url & url, const UTIL::LinkedStringMap & headers, const std::string & content);
		static void testHttpError(int code);
	};
}

#endif
