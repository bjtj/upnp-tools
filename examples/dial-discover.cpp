#include <iostream>
#include <vector>
#include <libhttp-server/Url.hpp>
#include <libupnp-tools/HttpUtils.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>
#include <liboslayer/FileStream.hpp>
#include <liboslayer/Text.hpp>

using namespace std;
using namespace osl;
using namespace ssdp;
using namespace upnp;
using namespace http;

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
	const string applicationUrl() const {return _applicationUrl;}
	string toString() {
		return _device->friendlyName() + " (" + _applicationUrl + ")";
	}
	const bool operator== (const AutoRef<UPnPDevice> & device) {
		return _device == device;
	}
		
};


class DialLauncher
{
private:
	vector<DialDevice> _devices;
public:
    DialLauncher() {
	}
    virtual ~DialLauncher() {
	}
	vector<DialDevice> & devices() {
		return _devices;
	}
	void add(DialDevice & device) {
		_devices.push_back(device);
	}
	void remove(const AutoRef<UPnPDevice> & device) {
		for (vector<DialDevice>::iterator iter = _devices.begin(); iter != _devices.end(); iter++) {
			if (iter->device() == device) {
				_devices.erase(iter);
				break;
			}
		}
	}
	void launch(DialDevice & device, const string & appName) {
		string url = device.applicationUrl() + appName;
		string payload = "";
		try {
			cout << "[POST]" << endl << HttpUtils::httpRequest(Url(url), "POST", LinkedStringMap(), payload)
				.getResponseHeader().getStatusCode() << endl;
		} catch (Exception e) {
			cout << "POST/Err: " << e.toString() << endl;
		}
	}
	void status(DialDevice & device, const string & appName) {
		string url = device.applicationUrl() + appName;
		try {
			cout << "[GET]" << endl << HttpUtils::httpGet(Url(url)) << endl;
		} catch (Exception e) {
			cout << "GET/Err: " << e.toString() << endl;
		}
	}
	void stop(DialDevice & device, const string & appName) {
		string url = device.applicationUrl() + appName;
		try {
			cout << "[DELETE]" << endl << HttpUtils::httpRequest(Url(url), "DELETE")
				.getResponseHeader().getStatusCode() << endl;
		} catch (Exception e) {
			cout << "DELETE/Err: " << e.toString() << endl;
		}
	}

	DialDevice & operator[] (size_t idx) {
		return _devices[idx];
	}
};


class DeviceListener : public UPnPDeviceListener {
private:
	DialLauncher & _launcher;
public:
	DeviceListener(DialLauncher & launcher) : _launcher(launcher) {}
    virtual ~DeviceListener() {}
	virtual void onDeviceAdded(AutoRef<UPnPDevice> device) {
		if (device->meta()["Application-URL"].empty() == false) {
			DialDevice dialDevice(device, device->meta()["Application-URL"]);
			_launcher.add(dialDevice);
			cout << dialDevice.toString() << endl;
		}
	}
	virtual void onDeviceRemoved(AutoRef<UPnPDevice> device) {
		_launcher.remove(device);
		cout << " ** Removed: " << device->friendlyName() << endl;
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

	DialLauncher launcher;

	HttpUtils::setConnectionTimeout(30000);
	HttpUtils::setRecvTimeout(30000);

	UPnPControlPoint cp(UPnPControlPoint::Config(9998));
	cp.setDeviceListener(AutoRef<UPnPDeviceListener>(new DeviceListener(launcher)));
	cp.startAsync();

	string dialServiceType = "urn:dial-multiscreen-org:service:dial:1";
	cp.sendMsearchAsync(dialServiceType, 3);

	cout << "(h or help)" << endl;

	while (1) {
		string line = readline();
		vector<string> toks = Text::split(line, " ");
		if (toks.empty()) {
			printList(launcher.devices());
			continue;
		}
		
		if (line == "q" || line == "quit") {
			break;
		} else if (line == "h" || line == "help") {
			cout << "[help] commands: h|help, q|quit, scan, launch, status, stop" << endl;
		} else if (line == "scan") {
			cp.sendMsearchAsync(dialServiceType, 3);
		} else if (toks[0] == "status") {
			int idx = Text::toInt(toks[1]) - 1;
			string appName = toks[2];
			launcher.status(launcher[idx], appName);
		} else if (toks[0] == "launch") {
			int idx = Text::toInt(toks[1]) - 1;
			string appName = toks[2];
			launcher.launch(launcher[idx], appName);
		} else if (toks[0] == "stop") {
			int idx = Text::toInt(toks[1]) - 1;
			string appName = toks[2];
			launcher.stop(launcher[idx], appName);
		} else {
			cout << "No operation -- '" << line << "'" << endl;
		}
	}

	cp.stop();
	cout << "BYE." << endl;
    return 0;
}
