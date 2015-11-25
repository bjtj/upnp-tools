#include <iostream>
#include <libupnp-tools/Uuid.hpp>

using namespace std;
using namespace UPNP;

int main(int argc, char * args[]) {
    
    //string sample = "12345678-1234-abcd-ef12-1122aabbccdd";
	string sample = "2fac1234-31f8-11b4-a222-08002b34c003";
	
    
    Uuid::validFormat(sample);
    
    cout << "valid" << endl;

	getchar();
    
    return 0;
}