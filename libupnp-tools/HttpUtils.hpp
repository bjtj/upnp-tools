#ifndef __HTTP_UTILS_HPP__
#define __HTTP_UTILS_HPP__

#include <string>
#include <liboslayer/os.hpp>
#include <libhttp-server/AnotherHttpClient.hpp>
#include <libhttp-server/Url.hpp>
#include <liboslayer/StringElement.hpp>

namespace UPNP {

	class HttpUtils {
	private:

		class DumpResponseHandler : public HTTP::OnResponseListener {
		private:
			std::string dump;
		public:
			DumpResponseHandler() {
			}
			virtual ~DumpResponseHandler() {
			}
			virtual void onTransferDone(HTTP::HttpResponse & response, HTTP::DataTransfer * transfer, UTIL::AutoRef<HTTP::UserData> userData) {
				if (transfer) {
					dump = transfer->getString();
				}
			}
			virtual void onError(OS::Exception & e, UTIL::AutoRef<HTTP::UserData> userData) {
				printf("Error/e: %s\n", e.getMessage().c_str());
			}
			std::string & getDump() {
				return dump;
			}
		};
		
	public:
		HttpUtils();
		virtual ~HttpUtils();

		static std::string httpGet(const HTTP::Url & url) {

			HTTP::AnotherHttpClient client;
    
			DumpResponseHandler handler;
			client.setOnResponseListener(&handler);
    
			client.setFollowRedirect(true);
			client.setUrl(url);
			client.setRequest("GET", UTIL::LinkedStringMap(), NULL);
			client.execute();

			return handler.getDump();
		}
	};
}

#endif
