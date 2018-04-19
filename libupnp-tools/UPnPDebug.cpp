#include "UPnPDebug.hpp"

namespace UPNP {

	using namespace std;
	using namespace OS;


	/**
	 * @brief
	 */

	UPnPDebugInfo::UPnPDebugInfo() : _creationTick(tick_milli()) {
	}
	
	UPnPDebugInfo::~UPnPDebugInfo() {
	}
	
	unsigned long & UPnPDebugInfo::creationTick() {
		return _creationTick;
	}
	
	unsigned long UPnPDebugInfo::creationTick() const {
		return _creationTick;
	}
	
	string & UPnPDebugInfo::tag() {
		return _tag;
	}

	string UPnPDebugInfo::tag() const {
		return _tag;
	}
	
	InetAddress & UPnPDebugInfo::from() {
		return _from;
	}

	InetAddress UPnPDebugInfo::from() const {
		return _from;
	}
	
	InetAddress & UPnPDebugInfo::to() {
		return _to;
	}

	InetAddress UPnPDebugInfo::to() const {
		return _to;
	}
	
	string & UPnPDebugInfo::packet() {
		return _packet;
	}

	string UPnPDebugInfo::packet() const {
		return _packet;
	}
	

	/**
	 * @brief 
	 */

	UPnPDebug::UPnPDebug() : _max_history(1024) {
	}

	UPnPDebug::UPnPDebug(size_t max_history) : _max_history(max_history) {
	}
	
	UPnPDebug::~UPnPDebug() {
	}
	
	void UPnPDebug::debug(const UPnPDebugInfo & info) {
		if (!_listener.nil()) {
			_listener->onDebugInfo(info);
		}
		if (_max_history <= _history.size()) {
			_history.pop_front();
		}
		_history.push_back(info);
	}

	void UPnPDebug::clear() {
		_history.clear();
	}

	deque<UPnPDebugInfo> & UPnPDebug::history() {
		return _history;
	}

	void UPnPDebug::setOnDebugInfoListener(AutoRef<OnDebugInfoListener> listener) {
		_listener = listener;
	}

	/**
	 * @brief 
	 */

	UPnPDebuggable::UPnPDebuggable() {
	}
	
	UPnPDebuggable::~UPnPDebuggable() {
	}

	void UPnPDebuggable::debug(const string & tag, const string & packet) {
		UPnPDebugInfo info;
		info.tag() = tag;
		info.packet() = packet;
		debug(info);
	}

	void UPnPDebuggable::debug(const string & tag, const string & packet,
							   const InetAddress & from, const InetAddress & to) {
		UPnPDebugInfo info;
		info.tag() = tag;
		info.packet() = packet;
		info.from() = from;
		info.to() = to;
		debug(info);
	}

	void UPnPDebuggable::debug(const UPnPDebugInfo & info) {
		if (!_debug.nil()) {
			_debug->debug(info);
		}
	}

	void UPnPDebuggable::setDebug(AutoRef<UPnPDebug> debug) {
		this->_debug = debug;
	}

	AutoRef<UPnPDebug> UPnPDebuggable::getDebug() {
		return _debug;
	}
}
