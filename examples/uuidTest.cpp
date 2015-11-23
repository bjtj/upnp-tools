#include <iostream>
#include <libupnp-tools/Uuid.hpp>

using namespace std;
using namespace UPNP;

int main(int argc, char * args[]) {
    
    string sample = "12345678-1234-abcd-ef12-1122aabbccdd";
    
    Uuid::validFormat(sample);
    
    cout << "valid" << endl;
    
    return 0;
}