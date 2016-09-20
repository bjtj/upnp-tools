#include <liboslayer/TestSuite.hpp>
#include <libupnp-tools/UPnPCache.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace UPNP;

class CacheTestCase : public TestCase {
public:
	CacheTestCase() : TestCase("cache test") {
	}
	virtual ~CacheTestCase() {
	}
	virtual void test() {
		UPnPCache cache;

		cache.prolong(1000);
		idle(500);
		ASSERT(cache.outdated(), ==, false);

		idle(1000);
		ASSERT(cache.outdated(), ==, true);

		cache.prolong(1000);
		ASSERT(cache.outdated(), ==, false);
		ASSERT(cache.lifetimeFull(), >, 1500);
		ASSERT(cache.lifetimeRecent(), <, 1500);
	}
};


int main(int argc, char *args[]) {

	TestSuite ts;
	ts.addTestCase(AutoRef<TestCase>(new CacheTestCase));

	TestReport report(ts.testAll());
	report.validate();
	
    return 0;
}