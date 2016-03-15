#include <iostream>
#include <libupnp-tools/NetworkUtil.hpp>
#include "utils.hpp"

using namespace std;
using namespace UPNP;

static void test_network() {
	OS::InetAddress addr = NetworkUtil::selectDefaultAddress();
	cout << addr.getHost() << endl;
}

int main(int argc, char *args[]) {

	test_network();
    
    return 0;
}
