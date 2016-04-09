#include "utils.hpp"
#include <libupnp-tools/UPnPLocationResolver.hpp>

using namespace std;
using namespace UPNP;

class ServiceLocationResolver : public UPnPLocationResolver {
private:
	string prefix;
public:
    ServiceLocationResolver(const string & prefix) : prefix(prefix) {}
    virtual ~ServiceLocationResolver() {}
	virtual string resolve(const string & loc) {
		return prefix + loc;
	}
};

static void test_location_resolver() {
	ServiceLocationResolver resolver("/scpd");
	resolver.resolve(""); // "/scpd.xml?udn=" + _udn + "&serviceType=" + dummy
}

int main(int argc, char *args[]) {
	test_location_resolver()
    return 0;
}
