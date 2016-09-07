#include "UPnPDebug.hpp"

namespace UPNP {

	using namespace std;

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
}
