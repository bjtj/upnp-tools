#include <iostream>
#include <string>
#include <map>
#include <liboslayer/Text.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>

using namespace std;
using namespace UTIL;
using namespace UPNP;

vector<UPnPDevice> devices;

class Selection {
private:
    UPnPDevice device;
    UPnPService service;
public:
    void select(UPnPDevice & device) {
        this->device = device;
    }
    void select(UPnPService & service) {
        this->service = service;
    }
};

Selection selection;

class MyDeviceAddRemoveHandle : public DeviceAddRemoveListener {
private:
public:
	MyDeviceAddRemoveHandle() {}
	virtual ~MyDeviceAddRemoveHandle() {}

	virtual void onDeviceAdded(UPnPControlPoint & cp, UPnPDevice & device) {
		// cout << " ++ " << device.getUdn() << " ** " << device.getFriendlyName() << endl;
		devices.push_back(device);
	}
	virtual void onDeviceRemoved(UPnPControlPoint & cp, UPnPDevice & device) {
		// cout << " -- " << device.getUdn() << " ** " << device.getFriendlyName() << endl;
		for (vector<UPnPDevice>::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			if (!iter->getUdn().compare(device.getUdn())) {
				devices.erase(iter);
				return;
			}
		}
	}
};

class MyInvokeActionResponseListener : public InvokeActionResponseListener {
	virtual void onActionResponse(ID_TYPE id, const UPnPActionRequest & actionRequest, const UPnPActionResponse & response) {
		cout << "# Action Result : " << actionRequest.getActionName() << endl;
		vector<string> names = response.getParameterNames();
		for (size_t i = 0; i < names.size(); i++) {
			string & name = names[i];
			string value = response.getParameter(name);
			cout << " - " << name << " : " << value << endl;
		}
	}
};

size_t readline(char * buffer, size_t max) {
    if (fgets(buffer, (int)max - 1, stdin)) {
        buffer[strlen(buffer) - 1] = 0;
        return strlen(buffer);
    }
    return 0;
}

static void s_show_device_list_with_selection_number() {
    
    cout << "== DEVICE LIST (count: " << devices.size() << ") ==" << endl;
    for (size_t i = 0; i < devices.size(); i++) {
        UPnPDevice & device = devices[i];
        cout << "[" << i << "] UDN: " << device.getUdn() << endl;
        cout << " > Friendly Name: " << device.getFriendlyName() << endl;
    }
    cout << endl;
}

static void s_send_action(UPnPControlPoint & cp) {
    
    bool done = false;
    
    while (!done) {
        s_show_device_list_with_selection_number();
        
        char buffer[1024] = {0,};
        if (cout << "device # ", readline(buffer, sizeof(buffer)) > 0) {
            
            if (!strcmp(buffer, "q")) {
                return;
            }
            
            int selection = Text::toInt(buffer);
            UPnPDevice & device = devices[selection];
            vector<UPnPService> services = device.getServicesRecursive();
            cout << "service - " << services.size() << endl;
            for (size_t i = 0; i < services.size(); i++) {
                UPnPService & service = services[i];
                cout << "[" << i << "] type: " << service.getServiceType() << endl;
            }
            if (cout << "service # ", readline(buffer, sizeof(buffer)) > 0) {
                
                if (!strcmp(buffer, "q")) {
                    return;
                }
                
                UPnPService & service = services[Text::toInt(buffer)];
                
                vector<UPnPAction> actions = service.getScpd().getActions();
                for (vector<UPnPAction>::iterator iter = actions.begin(); iter != actions.end(); iter++) {
                    cout << " - " << iter->getName() << endl;
                }
                
                if (cout << "action: ", readline(buffer, sizeof(buffer)) > 0) {
                    
                    if (!strcmp(buffer, "q")) {
                        return;
                    }
                    
                    string actionName = buffer;
                    
                    UPnPActionParameters in;
                    UPnPAction action = service.getScpd().getAction(buffer);
                    vector<UPnPActionArgument> arguments = action.getArguments();
                    for (vector<UPnPActionArgument>::iterator iter = arguments.begin(); iter != arguments.end(); iter++) {
                        if (iter->in()) {
                            cout << iter->getName();
                            
                            if (iter->hasAllowedValueList()) {
                                cout << "(Allowed: " << Text::join(iter->getAllowedValueList(), ", ") << ")";
                            }
                            cout << " : ";
                            
                            readline(buffer, sizeof(buffer));
                            in[iter->getName()] = buffer;
                        }
                    }
                    cp.invokeAction(service, actionName, in);
                    
                    getchar();
                }
            }
        }
    }
}

static int s_cmd_handler(const char * cmd, UPnPControlPoint & cp) {
    
    if (!*cmd) {
        return 0;
    }

	if (!strcmp(cmd, "quit") || !strcmp(cmd, "q")) {
		return -1;
	}

	if (!strcmp(cmd, "l") || !strcmp(cmd, "list")) {

		cout << "== DEVICE LIST (count: " << devices.size() << ") ==" << endl;
		for (size_t i = 0; i < devices.size(); i++) {
			UPnPDevice & device = devices[i];
			cout << "UDN: " << device.getUdn() << endl;
			cout << " > Friendly Name: " << device.getFriendlyName() << endl;
		}
		cout << endl;

		return 0;
	}

	if (!strcmp(cmd, "m") || !strcmp(cmd, "scan")) {
		cp.sendMsearch("upnp:rootdevice");
		return 0;
	}
    
    if (!strcmp(cmd, "a") || !strcmp(cmd, "action")) {
        s_send_action(cp);
    }

	return 0;
}

static void s_test_cp() {

	bool done = false;
	char buffer[1024] = {0,};

	
    UPnPControlPoint cp;
	UPnPControlPointSsdpNotifyFilter filter;
	filter.addFilterType("upnp:rootdevice");
	cp.setFilter(filter);

	MyDeviceAddRemoveHandle listener;
	cp.setDeviceAddRemoveListener(&listener);
    
    MyInvokeActionResponseListener actionResponseListener;
    cp.setInvokeActionResponseListener(&actionResponseListener);

    cp.startAsync();
    
    cout << "cp - start" << endl;
    
	while (!done) {
        
        if (cout << "CMD> ", readline(buffer, sizeof(buffer)) <= 0) {
            continue;
        }
        
		if (s_cmd_handler(buffer, cp) < 0) {
			done = true;
			break;
		}
	}

    cp.stop();
    
    cout << "cp - finish" << endl;
}

/**
 * main
 */
int main(int argc, char *args[]) {
	s_test_cp();
    return 0;
}
