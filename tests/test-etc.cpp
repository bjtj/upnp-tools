#include <liboslayer/TestSuite.hpp>
#include <libupnp-tools/UPnPUtils.hpp>
#include <libupnp-tools/UPnPExceptions.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace UPNP;

class MaxAgeTestCase : public TestCase {
public:
	MaxAgeTestCase() : TestCase("max-age test") {
	}
	virtual ~MaxAgeTestCase() {
	}
	virtual void test() {
		{
			MaxAge ma(1800);
			ASSERT(ma.second(), ==, 1800);
			ASSERT(ma.toString(), ==, "max-age=1800");
		}
		{
			MaxAge ma("max-age=1800");
			ASSERT(ma.second(), ==, 1800);
			ASSERT(ma.toString(), ==, "max-age=1800");
		}
		{
			MaxAge ma("MAX-AGE=1800");
			ASSERT(ma.second(), ==, 1800);
			ASSERT(ma.toString(), ==, "max-age=1800");
		}
		{
			MaxAge ma("Max-Age=1800");
			ASSERT(ma.second(), ==, 1800);
			ASSERT(ma.toString(), ==, "max-age=1800");
		}
		try {
			MaxAge ma("1800");
			throw "must be throw!";
		} catch (UPnPParseException e) {
			ASSERT(e.getMessage().size(), >, 0);
		}
	}
};


int main(int argc, char *args[]) {

	TestSuite ts;
	ts.addTestCase(AutoRef<TestCase>(new MaxAgeTestCase));

	TestReport report(ts.testAll());
	report.validate();
	
    return 0;
}
