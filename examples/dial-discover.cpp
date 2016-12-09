#include <iostream>
#include <vector>
#include <libhttp-server/Url.hpp>
#include <libupnp-tools/HttpUtils.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>
#include <liboslayer/FileStream.hpp>
#include <liboslayer/Text.hpp>

using namespace std;
using namespace OS;
using namespace SSDP;
using namespace UTIL;
using namespace UPNP;
using namespace HTTP;

class DialDevice {
private:
	AutoRef<UPnPDevice> _device;
	string _applicationUrl;
public:
    DialDevice(AutoRef<UPnPDevice> device, const string & applicationUrl)
		: _device(device), _applicationUrl(applicationUrl) { /**/ }
    virtual ~DialDevice() { /**/ }
	AutoRef<UPnPDevice> & device() {return _device;}
	string & applicationUrl() {return _applicationUrl;}
	string toString() {
		return _device->getFriendlyName() + " (" + _applicationUrl + ")";
	}
};

vector<DialDevice> dialDevices;

class DeviceListener : public DeviceAddRemoveListener {
private:
public:
	DeviceListener() {}
    virtual ~DeviceListener() {}
	virtual void onDeviceAdd(AutoRef<UPnPDevice> device) {
		if (device->meta()["Application-URL"].empty() == false) {
			DialDevice dialDevice(device, device->meta()["Application-URL"]);
			dialDevices.push_back(dialDevice);
			cout << dialDevice.toString() << endl;
		}
	}
	virtual void onDeviceRemove(AutoRef<UPnPDevice> device) {
		cout << " ** Removed: " << device->getFriendlyName() << endl;
	}
};

/**
 * @brief 
 */
string readline() {
	FileStream fs(stdin);
	return fs.readline();
}

/**
 * @brief 
 */
void printList(vector<DialDevice> & devices) {
	cout << "____" << endl;
	cout << "DIAL devices (cnt: " << devices.size() << ")" << endl;
	size_t i = 0;
	for (vector<DialDevice>::iterator iter = devices.begin(); iter != devices.end(); iter++, i++) {
		cout << " <|" << (i+1) << "| " << iter->toString() << " |>" << endl;
	}
}

/**
 * @brief 
 */
int main(int argc, char *args[]) {

	HttpUtils::setConnectionTimeout(30000);
	HttpUtils::setRecvTimeout(30000);

	UPnPControlPoint cp(UPnPControlPointConfig(9998));
	cp.setDeviceAddRemoveListener(AutoRef<DeviceAddRemoveListener>(new DeviceListener));
	cp.startAsync();

	cp.sendMsearchAsync("urn:dial-multiscreen-org:service:dial:1", 3);

	while (1) {
		string line = readline();
		vector<string> toks = Text::split(line, " ");
		if (toks.empty()) {
			printList(dialDevices);
			continue;
		}
		
		if (line == "q" || line == "quit") {
			break;
		} else if (line == "scan") {
			cp.sendMsearchAsync("urn:dial-multiscreen-org:service:dial:1", 3);
		} else if (toks[0] == "status") {
			int idx = Text::toInt(toks[1]) - 1;
			string appName = toks[2];
			string url = dialDevices[idx].applicationUrl() + appName;
			try {
				cout << "[GET]" << endl << HttpUtils::httpGet(Url(url)) << endl;
			} catch (Exception e) {
				cout << "GET/Err: " << e.getMessage() << endl;
			}
		} else if (toks[0] == "launch") {
			int idx = Text::toInt(toks[1]) - 1;
			string appName = toks[2];
			string url = dialDevices[idx].applicationUrl() + appName;
			string payload = "";
			try {
				cout << "[POST]" << endl << HttpUtils::httpRequest(Url(url), "POST", LinkedStringMap(), payload).getResponseHeader().getStatusCode() << endl;
			} catch (Exception e) {
				cout << "POST/Err: " << e.getMessage() << endl;
			}
		} else if (toks[0] == "stop") {
			int idx = Text::toInt(toks[1]) - 1;
			string appName = toks[2];
			string url = dialDevices[idx].applicationUrl() + appName;
			try {
				cout << "[DELETE]" << endl << HttpUtils::httpRequest(Url(url), "DELETE").getResponseHeader().getStatusCode() << endl;
			} catch (Exception e) {
				cout << "DELETE/Err: " << e.getMessage() << endl;
			}
		} else {
			cout << "no operation for '" << line << "'" << endl;
		}
	}

	cp.stop();
	cout << "BYE." << endl;
    return 0;
}
