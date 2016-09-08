#include "UPnPDebug.hpp"

namespace UPNP {

	using namespace std;
	using namespace UTIL;

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

	void UPnPDebug::setOnDebugInfoListener(UTIL::AutoRef<OnDebugInfoListener> listener) {
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

	void UPnPDebuggable::debug(const UPnPDebugInfo & info) {
		if (!_debug.nil()) {
			_debug->debug(info);
		}
	}

	void UPnPDebuggable::setDebug(AutoRef<UPnPDebug> debug) {
		this->_debug = debug;
	}
}
