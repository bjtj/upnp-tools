#ifndef __HTTP_UTILS_HPP__
#define __HTTP_UTILS_HPP__

#include <string>
#include <liboslayer/os.hpp>
#include <liboslayer/Text.hpp>
#include <libhttp-server/AnotherHttpClient.hpp>
#include <libhttp-server/HttpStatusCodes.hpp>
#include <libhttp-server/Url.hpp>
#include <liboslayer/StringElement.hpp>
#include <libhttp-server/FixedTransfer.hpp>

namespace UPNP {

	class HttpUtils {
	private:

		static unsigned long connectionTimeout;

		class DumpResponseHandler : public HTTP::OnResponseListener {
		private:
			HTTP::HttpResponseHeader responseHeader;
			std::string dump;
		public:
			DumpResponseHandler() {
			}
			virtual ~DumpResponseHandler() {
			}
			virtual void onTransferDone(HTTP::HttpResponse & response, HTTP::DataTransfer * transfer, UTIL::AutoRef<HTTP::UserData> userData) {
				responseHeader = response.getHeader();
				if (transfer) {
					dump = transfer->getString();
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
		static void testHttpError(int code);
	};
}

#endif
