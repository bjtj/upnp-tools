#ifndef __UPNP_DEBUG_HPP__
#define __UPNP_DEBUG_HPP__

#include <liboslayer/os.hpp>
#include <liboslayer/Network.hpp>
#include <liboslayer/AutoRef.hpp>
#include <string>
#include <deque>


namespace UPNP {


	/**
	 * @brief 
	 */
	class UPnPDebugInfo {
	private:
		unsigned long _creationTick;
		std::string _tag;
		OS::InetAddress _from;
		OS::InetAddress _to;
		std::string _packet;
		
	public:
		UPnPDebugInfo();
		virtual ~UPnPDebugInfo();
		unsigned long & creationTick();
		unsigned long creationTick() const;
		std::string & tag();
		std::string tag() const;
		OS::InetAddress & from();
		OS::InetAddress from() const;
		OS::InetAddress & to();
		OS::InetAddress to() const;
		std::string & packet();
		std::string packet() const;
	};

	
	/**
	 * @brief 
	 */
	class OnDebugInfoListener {
	public:
		OnDebugInfoListener() {}
		virtual ~OnDebugInfoListener() {}
		virtual void onDebugInfo(const UPnPDebugInfo & info) = 0;
	};


	/**
	 * @brief 
	 */
	class UPnPDebug {
	private:
		std::deque<UPnPDebugInfo> _history;
		OS::AutoRef<OnDebugInfoListener> _listener;
		size_t _max_history;
	public:
		UPnPDebug();
		UPnPDebug(size_t max_history);
		virtual ~UPnPDebug();
		void debug(const UPnPDebugInfo & info);
		void clear();
		std::deque<UPnPDebugInfo> & history();
		void setOnDebugInfoListener(OS::AutoRef<OnDebugInfoListener> listener);
	};

	/**
	 * @brief 
	 */
	class UPnPDebuggable {
	private:
		OS::AutoRef<UPnPDebug> _debug;
		
	public:
		UPnPDebuggable();
		virtual ~UPnPDebuggable();

		void debug(const std::string & tag, const std::string & packet);
		void debug(const std::string & tag, const std::string & packet,
				   const OS::InetAddress & from, const OS::InetAddress & to);
		void debug(const UPnPDebugInfo & info);
		void setDebug(OS::AutoRef<UPnPDebug> debug);
		OS::AutoRef<UPnPDebug> getDebug();
	};

}

#endif
