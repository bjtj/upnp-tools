#include "UPnPDebug.hpp"
#include <liboslayer/Logger.hpp>
#include <liboslayer/File.hpp>

namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace UTIL;


	static AutoRef<Logger> logger = LoggerFactory::instance().
		getObservingLogger(File::basename(__FILE__));

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


	UPnPDebug UPnPDebug::_instance;
	
	UPnPDebug::UPnPDebug() {
	}
	
	UPnPDebug::~UPnPDebug() {
	}
	
	UPnPDebug & UPnPDebug::instance() {
		return _instance;
	}
	
	void UPnPDebug::debug(const string & tag, const string & msg) {
		logger->debug("[" + tag + "]\n" + msg);
	}
}
