#include <iostream>
#include <string>
#include <map>
#include <liboslayer/Text.hpp>
#include <liboslayer/PollablePool.hpp>
#include <libupnp-tools/Timer.hpp>

using namespace std;
using namespace OS;
using namespace UPNP;
using namespace UTIL;

class MyTimerEvent : public TimerEvent {
private:
    int idx;
public:
    MyTimerEvent(bool heap) : TimerEvent(heap), idx(0) {}
    virtual ~MyTimerEvent() {}
    virtual void onFire() {
        cout << idx++ << "/fire!" << endl;
    }
};

class ToastTimerEvent : public TimerEvent {
private:
    string message;
public:
    ToastTimerEvent(bool heap, const string & message) : TimerEvent(heap), message(message) {}
    virtual ~ToastTimerEvent() {}
    
    virtual void onFire() {
        cout << " > msg : " << message << endl;
    }
};

class QuitTimerEvent : public TimerEvent {
private:
    bool * done;
public:
    QuitTimerEvent(bool heap, bool * done) : TimerEvent(heap), done(done) {}
    virtual ~QuitTimerEvent() {}
    
    virtual void onFire() {
        cout << "quit" << endl;
        *done = true;
    }
};

size_t readline(char * buffer, size_t max) {
    fgets(buffer, (int)max - 1, stdin);
    buffer[strlen(buffer) - 1] = 0;
    return strlen(buffer);
}

int main(int argc, char * args[]) {
    
    bool done = false;
    
    MyTimerEvent event(false);
    event.scheduleRepeatableFixedTimer(tick_milli(), -1, 5 * Timer::SECOND);
    
    Timer timer;
    timer.setTimerEvent(&event);
    
    LoopPoller poller;
    poller.registerPollee(&timer);
    PollingThread thread(&poller, 10);
    
    timer.start();
    thread.start();
    
    while (!done) {
        char buffer[1024] = {0,};
        if (readline(buffer, sizeof(buffer)) > 0) {
            if (Text::startsWith(buffer, "q")) {
                int sec = Text::toInt(buffer + 1);
                QuitTimerEvent * e = new QuitTimerEvent(true, &done);
                e->scheduleRelativeTimer(sec * Timer::SECOND);
                timer.setTimerEvent(e);
            }
            if (Text::startsWith(buffer, "e")) {
                
                int sec = Text::toInt(buffer + 1);
                
                ToastTimerEvent * e = new ToastTimerEvent(true, "hello~");
                e->scheduleRelativeTimer(sec * Timer::SECOND);
                timer.setTimerEvent(e);
            }
        }
    }
    
    timer.stop();
    thread.interrupt();
    thread.join();
    
    return 0;
}