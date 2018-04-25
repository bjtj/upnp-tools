#ifndef __SSDP_MULTICAST_LISTENER_HPP__
#define __SSDP_MULTICAST_LISTENER_HPP__

#include <string>
#include <liboslayer/os.hpp>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/DatagramSocket.hpp>
#include "SSDPEventListener.hpp"

namespace ssdp {

	class SSDPMulticastListener {
	private:
		std::string group;
		osl::MulticastSocket sock;
		osl::AutoRef<osl::Selector> selector;
		std::vector<osl::AutoRef<SSDPEventListener> > listeners;

	public:
		SSDPMulticastListener(const std::string & group, int port);
		SSDPMulticastListener(const std::string & group, int port, osl::AutoRef<osl::Selector> selector);
		virtual ~SSDPMulticastListener();

		void start();
		void stop();
		bool isRunning();
		void poll(unsigned long timeout);
		bool isReadable();
		bool isReadable(osl::Selector & selector);
		void procRead();
		void onReceive(osl::DatagramPacket & packet);
		void addSSDPEventListener(osl::AutoRef<SSDPEventListener> listener);
		void removeSSDPEventListener(osl::AutoRef<SSDPEventListener> listener);
	};
}

#endif
