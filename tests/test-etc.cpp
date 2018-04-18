#include <liboslayer/TestSuite.hpp>
#include <libupnp-tools/UPnPTerms.hpp>
#include <libupnp-tools/UPnPExceptions.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace UPNP;

/**
 * max-age test case
 */
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
			MaxAge ma = MaxAge::fromString("max-age=1800");
			ASSERT(ma.second(), ==, 1800);
			ASSERT(ma.toString(), ==, "max-age=1800");
		}
		{
			MaxAge ma = MaxAge::fromString("MAX-AGE=1800");
			ASSERT(ma.second(), ==, 1800);
			ASSERT(ma.toString(), ==, "max-age=1800");
		}
		{
			MaxAge ma = MaxAge::fromString("Max-Age=1800");
			ASSERT(ma.second(), ==, 1800);
			ASSERT(ma.toString(), ==, "max-age=1800");
		}
		try {
			MaxAge ma = MaxAge::fromString("1800");
			throw "must be throw!";
		} catch (UPnPParseException e) {
			ASSERT(e.toString().size(), >, 0);
		}
	}
};

/**
 * callback url test case
 */
class CallbackUrlsTestCase : public TestCase
{
public:
    CallbackUrlsTestCase() : TestCase("callback-urls-test-case") {}
    virtual ~CallbackUrlsTestCase() {}
	virtual void test() {
		{
			CallbackUrls cbs = CallbackUrls::fromString("<http://a>");
			ASSERT(cbs.urls().size(), ==, 1);
			ASSERT(cbs.urls()[0], ==, "http://a");
		}

		{
			try {
				CallbackUrls cbs = CallbackUrls::fromString("<>");
				throw "It should not be thrown!";
			} catch (UPnPParseException e) {
				cerr << "[expected exception] " << e.toString() << endl;
				ASSERT(e.toString().size(), >, 0);
			}
		}

		{
			try {
				CallbackUrls cbs = CallbackUrls::fromString("<x>");
				throw "It should not be thrown!";
			} catch (UPnPParseException e) {
				cerr << "[expected exception] " << e.toString() << endl;
				ASSERT(e.toString().size(), >, 0);
			}
		}

		{
			CallbackUrls cbs = CallbackUrls::fromString("<http://a> <http://b>");
			ASSERT(cbs.urls().size(), ==, 2);
			ASSERT(cbs.urls()[0], ==, "http://a");
			ASSERT(cbs.urls()[1], ==, "http://b");
		}
	}
};

/**
 * Second- test case
 */
class SecondTestCase : public TestCase {
public:
    SecondTestCase() : TestCase("second-test-case") {}
    virtual ~SecondTestCase() {}
	virtual void test() {
		Second second = Second::fromString("Second-100");
		ASSERT(second.second(), ==, 100);

		second = Second(140);
		ASSERT(second.toString(), ==, "Second-140");

		try {
			Second::fromString("120");
			throw "It should not be thrown!";
		} catch (UPnPParseException & e) {
			ASSERT(e.toString().size(), >, 0);
		}
	}
};


/**
 * main
 */
int main(int argc, char *args[]) {

	TestSuite ts;
	ts.addTestCase(AutoRef<TestCase>(new MaxAgeTestCase));
	ts.addTestCase(AutoRef<TestCase>(new CallbackUrlsTestCase));
	ts.addTestCase(AutoRef<TestCase>(new SecondTestCase));

	TestReport report(ts.testAll());
	report.validate();
	
    return 0;
}
