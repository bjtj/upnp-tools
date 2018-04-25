#include "utils.hpp"
#include <liboslayer/os.hpp>
#include <libupnp-tools/UPnPResourceManager.hpp>
#include <liboslayer/FileStream.hpp>
#include <libhttp-server/Url.hpp>

using namespace std;
using namespace osl;
using namespace http;
using namespace upnp;


static void test_resource_manager() {

	string cwd = File::getCwd();
	
	FileStream fs("test.txt", "wb+");
	string text = "hello world";
	fs.write(text.c_str(), text.size());
	fs.close();

	ASSERT(Url("file://" + cwd + "/test.txt").getPath(), ==, cwd + "/test.txt");
	UPnPResourceManager & resMan = UPnPResourceManager::instance();
	string data = resMan.getResourceContent(Url("file://" + cwd + "/test.txt"));
	
	ASSERT(data, ==, text);

	Url base("file:///base/path/index.txt");
	Url relative = base.relativePath("proj/hello.txt");
	ASSERT(relative.getPath(), ==, "/base/path/proj/hello.txt");
}

static void test_web_resource() {
	UPnPResourceManager & resMan = UPnPResourceManager::instance();
	string dump = resMan.getResourceContent(Url("http://www.google.com"));
	cout << dump << endl;
}

int main(int argc, char *args[]) {

	test_resource_manager();
	test_web_resource();
    
    return 0;
}
