#include <iostream>
#include <libupnp-tools/UPnPServer.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace UPNP;

int main(int argc, char * args[]) {
    
    UPnPServer server(8080);
    
    LinkedStringMap props;
    props["ContainerUpdateIDs"] = "";
    props["SystemUpdateID"] = "0";
    
    string str = server.toPropertySetXmlString(props);
    
    cout << str << endl;
    
    cout << "" << endl;
    
    UPnPControlPoint cp(8081);
    
    LinkedStringMap values = cp.parsePropertySetXmlString(str);
    
    for (size_t i = 0; i < values.size(); i++) {
        NameValue & nv = values.getByIndex(i);
        cout << nv.getName() << " : " << nv.getValue() << endl;
    }
    
    return 0;
}