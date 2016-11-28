#include "SSDPServer.hpp"
#include "SSDPMsearchSender.hpp"

namespace SSDP {

	using namespace std;
	using namespace OS;
	using namespace UTIL;

	static const char * MCAST_HOST = "239.255.255.250";
	static const int MCAST_PORT = 1900;

	/**
	 * @brief 
	 */
	class PollingThread : public Thread {
	private:
		SSDPServer & server;
		unsigned long timeout;
	public:
		PollingThread(SSDPServer & server, unsigned long timeout) : server(server), timeout(timeout) {}
		virtual ~PollingThread() {}
		virtual void run() {
			while (!interrupted()) {
				server.poll(timeout);
			}
		}
	};

	/**
	 * @brief 
	 */
	class MSearchSession {
	private:
		AutoRef<SSDPMsearchSender> sender;
		unsigned long startTick;
		unsigned long timeout;
	public:
		MSearchSession(AutoRef<SSDPMsearchSender> sender, unsigned long timeout)
			: sender(sender), startTick(tick_milli()), timeout(timeout) {
		}
		virtual ~MSearchSession() {
		}
		bool testTimeout() {
			return (tick_milli() - startTick >= timeout);
		}
		void doProc() {
			if (sender->isReadable()) {
				sender->procRead();
			}
		}
		void close() {
			sender->close();
		}
		bool equalsSender(const AutoRef<SSDPMsearchSender> & sender) {
			return this->sender == sender;
		}
	};


	/**
	 * @brief 
	 */
	class MSearchResponseListenerThread : public Thread {
	private:
		Semaphore sem;
		AutoRef<Selector> selector;
		unsigned long selectTimeout;
		vector<MSearchSession> sessions;
	public:
		MSearchResponseListenerThread(const AutoRef<Selector> selector, unsigned long selectTimeout)
			: sem(1), selector(selector), selectTimeout(selectTimeout) {
		}
		virtual ~MSearchResponseListenerThread() {
		}
		virtual void run() {
			while (!interrupted()) {
				if (selector->select(selectTimeout) > 0) {
					sem.wait();
					for (vector<MSearchSession>::iterator iter = sessions.begin(); iter != sessions.end();) {
						if (iter->testTimeout()) {
							iter->close();
							iter = sessions.erase(iter);
						} else {
							iter->doProc();
							iter++;
						}
					}
					sem.post();
				} else {
					sem.wait();
					for (vector<MSearchSession>::iterator iter = sessions.begin(); iter != sessions.end();) {
						if (iter->testTimeout()) {
							iter->close();
							iter = sessions.erase(iter);
						} else {
							iter++;
						}
					}
					sem.post();
				}
			}
		}
		void registerSession(const MSearchSession & session) {
			sem.wait();
			sessions.push_back(session);
			sem.post();
		}
		void unregisterSession(AutoRef<SSDPMsearchSender> sender) {
			sem.wait();
			for (vector<MSearchSession>::iterator iter = sessions.begin(); iter != sessions.end();) {
				if (iter->equalsSender(sender)) {
					iter = sessions.erase(iter);
				} else {
					iter++;
				}
			}
			sem.post();
		}
		AutoRef<Selector> getSelector() {
			return selector;
		}
	};

	

	SSDPServer::SSDPServer()
		: selector(new SharedSelector),
		  mcastListener(MCAST_HOST, MCAST_PORT),
		  pollingThread(NULL),
		  msearchResponseListenerThread(NULL){
	}
	
	SSDPServer::~SSDPServer() {
	}

	void SSDPServer::start() {
		mcastListener.start();
	}
	
	void SSDPServer::startAsync() {
		start();
		startPollingThread();
	}
	void SSDPServer::stop() {
		stopMSearchResponseListenerThread();
		stopPollingThread();
		mcastListener.stop();
	}
    void SSDPServer::poll(unsigned long timeout) {
        mcastListener.poll(timeout);
    }
	
	bool SSDPServer::isPollingThreadRunning() {
		return (pollingThread != NULL);
	}
	void SSDPServer::startPollingThread() {
		if (!isPollingThreadRunning()) {
			pollingThread = new PollingThread(*this, 100);
			pollingThread->start();
		}
	}
	void SSDPServer::stopPollingThread() {
		if (isPollingThreadRunning()) {
			pollingThread->interrupt();
			pollingThread->join();
			delete pollingThread;
			pollingThread = NULL;
		}
	}
	
	bool SSDPServer::isMSearchResponseListenerThreadRunning() {
		return (msearchResponseListenerThread != NULL);
	}
	void SSDPServer::startMSearchResponseListenerThread() {
		if (!isMSearchResponseListenerThreadRunning()) {
			msearchResponseListenerThread = new MSearchResponseListenerThread(selector, 100);
			msearchResponseListenerThread->start();
		}
	}
	void SSDPServer::stopMSearchResponseListenerThread() {
		if (isMSearchResponseListenerThreadRunning()) {
			msearchResponseListenerThread->interrupt();
			msearchResponseListenerThread->join();
			delete msearchResponseListenerThread;
			msearchResponseListenerThread = NULL;
		}
	}

	void SSDPServer::supportMsearchAsync(bool support) {
		if (support) {
			startMSearchResponseListenerThread();
		} else {
			stopMSearchResponseListenerThread();
		}
	}
	
	void SSDPServer::sendMsearchAndGather(const string & st, unsigned long timeoutSec) {
		AutoRef<SSDPMsearchSender> sender = sendMsearch(st, timeoutSec);
		sender->gather(timeoutSec * 1000);
		sender->close();
	}
	void SSDPServer::sendMsearchAndGather(vector<string> & st, unsigned long timeoutSec) {
		AutoRef<SSDPMsearchSender> sender = sendMsearch(st, timeoutSec);
		sender->gather(timeoutSec * 1000);
		sender->close();
	}
	void SSDPServer::sendMsearchAsync(const string & st, unsigned long timeoutSec) {

		if (!isMSearchResponseListenerThreadRunning()) {
			throw Exception("Error: background service should be running first");
		}

		AutoRef<Selector> selector = ((MSearchResponseListenerThread*)msearchResponseListenerThread)->getSelector();
		AutoRef<SSDPMsearchSender> sender = sendMsearch(st, timeoutSec, selector);
		((MSearchResponseListenerThread*)msearchResponseListenerThread)->registerSession(MSearchSession(sender, timeoutSec * 1000));
	}
	void SSDPServer::sendMsearchAsync(const vector<string> & st, unsigned long timeoutSec) {

		if (!isMSearchResponseListenerThreadRunning()) {
			throw Exception("Error: background service should be running first");
		}

		AutoRef<Selector> selector = ((MSearchResponseListenerThread*)msearchResponseListenerThread)->getSelector();
		AutoRef<SSDPMsearchSender> sender = sendMsearch(st, timeoutSec, selector);
		((MSearchResponseListenerThread*)msearchResponseListenerThread)->registerSession(MSearchSession(sender, timeoutSec * 1000));
	}
	AutoRef<SSDPMsearchSender> SSDPServer::sendMsearch(const string & st, unsigned long timeoutSec) {
		return sendMsearch(st, timeoutSec, selector);
	}
	AutoRef<SSDPMsearchSender> SSDPServer::sendMsearch(const string & st, unsigned long timeoutSec, AutoRef<Selector> selector) {
		AutoRef<SSDPMsearchSender> sender(new SSDPMsearchSender(selector));
		sender->addSSDPEventListener(listener);
		sender->sendMsearchAllInterfaces(st, timeoutSec, MCAST_HOST, MCAST_PORT);
		return sender;
	}
	AutoRef<SSDPMsearchSender> SSDPServer::sendMsearch(const vector<string> & st, unsigned long timeoutSec) {
		return sendMsearch(st, timeoutSec, selector);
	}
	AutoRef<SSDPMsearchSender> SSDPServer::sendMsearch(const vector<string> & st, unsigned long timeoutSec, AutoRef<Selector> selector) {
		AutoRef<SSDPMsearchSender> sender(new SSDPMsearchSender(selector));
		sender->addSSDPEventListener(listener);
		for (vector<string>::const_iterator iter = st.begin(); iter != st.end(); iter++) {
			sender->sendMsearchAllInterfaces(*iter, timeoutSec, MCAST_HOST, MCAST_PORT);
		}
		return sender;
	}
	void SSDPServer::addSSDPEventListener(AutoRef<SSDPEventListener> listener) {
		this->listener = listener;
		mcastListener.addSSDPEventListener(listener);
	}

}
