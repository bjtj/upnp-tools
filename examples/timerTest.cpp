#include <iostream>
#include <string>
#include <map>
#include <liboslayer/Text.hpp>
#include <libupnp-tools/Timer.hpp>

using namespace std;
using namespace OS;
using namespace UPNP;

bool done = false;

class MyTimerEvent : public TimerEvent {
private:
    int idx;
public:
    MyTimerEvent() {}
    virtual ~MyTimerEvent() {}
    virtual void onFire() {
        cout << idx++ << "/fire!" << endl;
    }
};

class MyTimerListener : public TimerListener {
private:
public:
    MyTimerListener() {}
    virtual ~MyTimerListener() {}
    virtual void onTimerEventDone(TimerEvent * event) {
        done = true;
    }
};

int main(int argc, char * args[]) {
    
    MyTimerEvent event;
    event.scheduleTimer(tick_milli(), -1, 200UL);
    
    MyTimerListener listener;
    
    Timer timer;
    timer.setTimerEvent(&event);
    timer.setTimerListener(&listener);
    
    while (!done) {
        timer.poll();
    }
    
    return 0;
}