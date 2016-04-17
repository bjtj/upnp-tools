#include "SSDPServer.hpp"
#include "SSDPMsearchSender.hpp"

namespace SSDP {

	static const char * MCAST_HOST = "239.255.255.250";
	static const int MCAST_PORT = 1900;

	class PollThread : public OS::Thread {
	private:
		SSDPServer & server;
		unsigned long timeout;
	public:
		PollThread(SSDPServer & server, unsigned long timeout) : server(server), timeout(timeout) {}
		virtual ~PollThread() {}
		virtual void run() {
			while (!interrupted()) {
				server.poll(timeout);
			}
		}
	};


	SSDPServer::SSDPServer() : mcastListener(MCAST_HOST, MCAST_PORT), thread(NULL) {
	}
	SSDPServer::~SSDPServer() {
	}

	void SSDPServer::start() {
		mcastListener.start();
	}
	void SSDPServer::startAsync() {
		start();
		if (!thread) {
			thread = new PollThread(*this, 100);
			thread->start();
		}
	}
	void SSDPServer::stop() {
		if (thread) {
			thread->interrupt();
			thread->join();
			delete thread;
			thread = NULL;
		}
		mcastListener.stop();
	}
    void SSDPServer::poll(unsigned long timeout) {
        mcastListener.poll(timeout);
    }
	void SSDPServer::sendMsearchAndGather(const std::string & st, unsigned long timeoutSec) {
		UTIL::AutoRef<SSDPMsearchSender> sender = sendMsearch(st, timeoutSec);
		sender->gather(timeoutSec * 1000);
		sender->close();
	}
	void SSDPServer::sendMsearchAndGather(std::vector<std::string> & st, unsigned long timeoutSec) {
		UTIL::AutoRef<SSDPMsearchSender> sender = sendMsearch(st, timeoutSec);
		sender->gather(timeoutSec * 1000);
		sender->close();
	}
	UTIL::AutoRef<SSDPMsearchSender> SSDPServer::sendMsearch(const std::string & st, unsigned long timeoutSec) {
		UTIL::AutoRef<SSDPMsearchSender> sender(new SSDPMsearchSender);
		sender->addSSDPEventHandler(handler);
		sender->sendMsearchAllInterfaces(st, timeoutSec, MCAST_HOST, MCAST_PORT);
		return sender;
	}
	UTIL::AutoRef<SSDPMsearchSender> SSDPServer::sendMsearch(std::vector<std::string> & st, unsigned long timeoutSec) {
		UTIL::AutoRef<SSDPMsearchSender> sender(new SSDPMsearchSender);
		sender->addSSDPEventHandler(handler);
		for (std::vector<std::string>::iterator iter = st.begin(); iter != st.end(); iter++) {
			sender->sendMsearchAllInterfaces(*iter, timeoutSec, MCAST_HOST, MCAST_PORT);
		}
		return sender;
	}
	void SSDPServer::addSSDPEventHandler(UTIL::AutoRef<SSDPEventHandler> handler) {
		this->handler = handler;
		mcastListener.addSSDPEventHandler(handler);
	}

}
