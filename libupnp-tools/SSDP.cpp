#include "SSDP.hpp"

#include <liboslayer/Text.hpp>

namespace upnp {

	using namespace std;
	using namespace osl;

	string SSDP::GROUP = "239.255.255.250";
	int SSDP::PORT = 1900;

	string SSDP::host() {
		return GROUP + ":" + Text::toString(PORT);
	}
	
}
