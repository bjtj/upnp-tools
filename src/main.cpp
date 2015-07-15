#include <iostream>
#include "SSDPServer.hpp"
#include "os.hpp"

using namespace std;
using namespace SSDP;

static void s_test_ssdp_server() {
	SSDPServer server;

	server.startAsync();

	getchar();

	server.stop();
}

int main(int argc, char *args[]) {

	s_test_ssdp_server();

	std::cout << "Done" << std::endl;
    
    return 0;
}
