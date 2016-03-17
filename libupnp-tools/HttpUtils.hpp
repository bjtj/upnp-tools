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
		HttpUtils() {}
		virtual ~HttpUtils() {}

		static DumpResponseHandler dumpHttpRequest(const HTTP::Url & url, const std::string & method, const UTIL::LinkedStringMap & headers) {
			
			HTTP::AnotherHttpClient client;
    
			DumpResponseHandler handler;
			client.setOnResponseListener(&handler);
    
			client.setFollowRedirect(true);
			client.setUrl(url);
			client.setRequest(method, headers, NULL);
			client.execute();

			return handler;
		}

		static std::string httpGet(const HTTP::Url & url) {

			HTTP::AnotherHttpClient client;
    
			DumpResponseHandler handler;
			client.setOnResponseListener(&handler);
    
			client.setFollowRedirect(true);
			client.setUrl(url);
			client.setRequest("GET", UTIL::LinkedStringMap(), NULL);
			client.execute();

			int code = handler.getResponseHeader().getStatusCode();
			if (code / 100 != 2) {
				std::string codeString = UTIL::Text::toString(code);
				std::string msg = HTTP::HttpStatusCodes::getMessage(code);
				throw OS::Exception("http error - " + codeString + " / " + msg, -1, 0);
			}

			return handler.getDump();
		}

		static std::string httpPost(const HTTP::Url & url, const UTIL::LinkedStringMap & headers, const std::string & content) {
			HTTP::AnotherHttpClient client;
			DumpResponseHandler handler;
			client.setOnResponseListener(&handler);
			client.setFollowRedirect(true);
			client.setUrl(url);
			client.setRequest("POST", headers, new HTTP::FixedTransfer(content));
			client.execute();
			return handler.getDump();
		}
	};
}

#endif
