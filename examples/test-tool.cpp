/**
 * spec
 * ----
 * discovery
 * description
 * scpd
 * action
 * event
 * error handling
 * timeout check
 */
#include <string>
#include <libupnp-tools/UPnPControlPoint.hpp>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/FileStream.hpp>
#include <liboslayer/Properties.hpp>

using namespace std;
using namespace UTIL;
using namespace UPNP;

/**
 *
 */
class TestEnvironment : public Properties {
private:
public:
	TestEnvironment() {}
	virtual ~TestEnvironment() {}
};

/**
 *
 */
class TestSuite {
private:
	AutoRef<TestEnvironment> _env;
	string _name;
public:
	TestSuite(const string & name) : _name(name) {}
	virtual ~TestSuite() {}
	AutoRef<TestEnvironment> env() {return _env;}
	virtual void setUp() = 0;
	virtual void tearDown() = 0;
	virtual void test() = 0;
};

/**
 *
 */
class TestResult {
private:
	bool _result;
	stirng _message;
public:
	TestResult(bool result, const string & message) : _result(result), _message(message) {}
	virtual ~TestResult() {}
	bool & result() {return _result;}
	string & message() {return _message;}
};

/**
 *
 */
class TestRunner {
private:
	AutoRef<TestEnvironment> _env;
	vector<AutoRef<TestSuite> > _suites;
	vector<TestResult> _results;
public:
	TestRunner() {}
	virtual ~TestRunner() {}
	vector<AutoRef<TestSuite> > suites() {
		return _suites;
	}
	void addSuite(AutoRef<TestSuite> suite) {
		_suites.push_back(suite);
	}
	AutoRef<TestEnvironment> env() {
		return _env;
	}
	vector<TestResult> runAll() {
		_results.clear();
		for (vector<AutoRef<TestSuite> >::iterator iter = _suites.begin(); iter != _suites.end(); iter++) {
			test(*iter);
		}
		return _results;
	}
	TestResult test(AutoRef<TestSuite> suite) {
		try {
			suite->test();
			return TextResult(true, "OK");
		} catch (Exception e) {
			return TestResult(false, e.getMessage());
		}
	}
	vector<TestResult> results() {
		return _results;
	}
	void report(const TestResult & result) {
		_results.push_back(result);
	}
};


class TestTarget {
private:
	AutoRef<UPnPDevice> _device;
public:
    TestTarget() {
	}
    virtual ~TestTarget() {
	}
	AutoRef<UPnPDevice> & device() {
		return _device;
	}
};

static void s_print_service_oneline(AutoRef<UPnPService> service) {
	cout << "Service Type: " << service->getServiceType() << endl;
}

static void s_print_device_oneline(AutoRef<UPnPDevice> device) {
	cout << "Device : " << device->getUdn() << " - " << device->getFriendlyName() << endl;
}

static AutoRef<UPnPDevice> s_select_target_device(UPnPControlPoint & cp) {
	FileStream fs(stdin);
	while (1) {
		string lline = fs.readline();
		if (line == "q" || line == "quit") {
			return NULL;
		}
	}
}

int main(int argc, char *args[]) {

	TestTarget target;

	// 1. select target device
	// 2. start test
    
    return 0;
}

