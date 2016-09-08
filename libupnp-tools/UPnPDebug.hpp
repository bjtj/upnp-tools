#ifndef __UPNP_DEBUG_HPP__
#define __UPNP_DEBUG_HPP__

#include <liboslayer/os.hpp>
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
		UPnPDebugInfo() : _creationTick(OS::tick_milli()) {}
		virtual ~UPnPDebugInfo() {}
		unsigned long & creationTick() { return _creationTick; }
		std::string & tag() { return _tag; }
		OS::InetAddress & from() { return _from; }
		OS::InetAddress & to() { return _to; }
		std::string & packet() { return _packet; }

		unsigned long const_creationTick() const { return _creationTick; }
		std::string const_tag() const { return _tag; }
		OS::InetAddress const_from() const { return _from; }
		OS::InetAddress const_to() const { return _to; }
		std::string const_packet() const { return _packet; }
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
		UTIL::AutoRef<OnDebugInfoListener> _listener;
		size_t _max_history;
	public:
		UPnPDebug();
		UPnPDebug(size_t max_history);
		virtual ~UPnPDebug();
		void debug(const UPnPDebugInfo & info);
		void clear();
		std::deque<UPnPDebugInfo> & history();
		void setOnDebugInfoListener(UTIL::AutoRef<OnDebugInfoListener> listener);
	};

	/**
	 * @brief 
	 */
	class UPnPDebuggable {
	private:
		UTIL::AutoRef<UPnPDebug> _debug;
		
	public:
		UPnPDebuggable();
		virtual ~UPnPDebuggable();

		void debug(const std::string & tag, const std::string & packet);
		void debug(const UPnPDebugInfo & info);
		void setDebug(UTIL::AutoRef<UPnPDebug> debug);
	};

}

#endif
