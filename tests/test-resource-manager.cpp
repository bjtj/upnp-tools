#include "utils.hpp"
#include <liboslayer/os.hpp>
#include <libupnp-tools/UPnPResourceManager.hpp>
#include <liboslayer/FileStream.hpp>
#include <libhttp-server/Url.hpp>

using namespace std;
using namespace OS;
using namespace UPNP;
using namespace HTTP;
using namespace UTIL;

static void test_resource_manager() {

	string cwd = File::getCwd();
	
	FileStream fs("test.txt", "wb+");
	string text = "hello world";
	fs.write(text.c_str(), text.size());
	fs.close();

	ASSERT(Url("file://" + cwd + "/test.txt").getPath(), ==, cwd + "/test.txt");
	
	string data = UPnPResourceManager::getResource(Url("file://" + cwd + "/test.txt"));
	
	ASSERT(data, ==, text);

	Url base("file:///base/path/index.txt");
	Url relative = base.relativePath("proj/hello.txt");
	ASSERT(relative.getPath(), ==, "/base/path/proj/hello.txt");
}

int main(int argc, char *args[]) {

	test_resource_manager();
    
    return 0;
}
