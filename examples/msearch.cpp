#include <iostream>
#include <liboslayer/Text.hpp>
#include <liboslayer/Date.hpp>
#include <libupnp-tools/SSDPMsearchSender.hpp>
#include <libupnp-tools/SSDPEventListener.hpp>

using namespace std;
using namespace osl;
using namespace ssdp;


class SsdpHandler : public SSDPEventListener {
private:
    int _count;
public:
    SsdpHandler() : _count(0) {}
    virtual ~SsdpHandler() {}
    virtual void onMsearchResponse(const SSDPHeader & header) {
	cout << "DATE: " << Date::now().toString() << endl;
	cout << "COUNT: " << ++_count << endl;
	cout << "FROM: " << header.getRemoteAddr().toString() << endl;
	cout << "----" << endl;
	cout << header.toString() << endl;
    }

    int count() {
	return _count;
    }
};


int main(int argc, char *argv[])
{
    string st = "ssdp:all";
    int mx = 3;
    if (argc > 1) {
	st = string(argv[1]);
    }
    if (argc > 2) {
	mx = Text::toInt(argv[2]);
    }

    cout << "Send msearch -- '" << st << "' with timeout " << mx << "sec." << endl;

    unsigned long tick = tick_milli();
	
    SSDPMsearchSender sender;
    SsdpHandler * handler = new SsdpHandler;
    AutoRef<SSDPEventListener> listener(handler);
    sender.addSSDPEventListener(listener);
    sender.sendMsearch(st, mx);
    sender.gather();
    sender.close();

    cout << "Elapsed: " << tick_milli() - tick << " ms." << endl;
    cout << "Total count: " << handler->count() << endl;
    
    return 0;
}
