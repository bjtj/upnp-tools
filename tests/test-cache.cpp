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

		cache.updateTime();
		cache.timeout() = 1000;
		idle(500);
		ASSERT(cache.expired(), ==, false);

		idle(1000);
		ASSERT(cache.expired(), ==, true);

		cache.updateTime();
		cache.timeout() = 1000;
		ASSERT(cache.expired(), ==, false);
	}
};


int main(int argc, char *args[]) {

	TestSuite ts;
	ts.addTestCase(AutoRef<TestCase>(new CacheTestCase));

	TestReport report(ts.testAll());
	report.validate();
	
    return 0;
}
