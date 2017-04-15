#ifndef __SSDP_MULTICAST_LISTENER_HPP__
#define __SSDP_MULTICAST_LISTENER_HPP__

#include <string>
#include <liboslayer/os.hpp>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/DatagramSocket.hpp>
#include "SSDPEventListener.hpp"

namespace SSDP {

	class SSDPMulticastListener {
	private:
		std::string group;
		OS::MulticastSocket sock;
		OS::AutoRef<OS::Selector> selector;
		std::vector<OS::AutoRef<SSDPEventListener> > listeners;

	public:
		SSDPMulticastListener(const std::string & group, int port);
		SSDPMulticastListener(const std::string & group, int port, OS::AutoRef<OS::Selector> selector);
		virtual ~SSDPMulticastListener();

		void start();
		void stop();
		bool isRunning();
		void poll(unsigned long timeout);
		bool isReadable();
		bool isReadable(OS::Selector & selector);
		void procRead();
		void onReceive(OS::DatagramPacket & packet);
		void addSSDPEventListener(OS::AutoRef<SSDPEventListener> listener);
		void removeSSDPEventListener(OS::AutoRef<SSDPEventListener> listener);
	};
}

#endif
