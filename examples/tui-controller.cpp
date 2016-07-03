#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/Text.hpp>
#include <liboslayer/FileStream.hpp>
#include <liboslayer/XmlParser.hpp>
#include <libupnp-tools/UPnPControlPoint.hpp>
#include <libupnp-tools/UPnPActionInvoker.hpp>
#include <libupnp-tools/UPnPActionRequest.hpp>
#include <libupnp-tools/UPnPActionResponse.hpp>
#include <libupnp-tools/NetworkUtil.hpp>
#include <libupnp-tools/UPnPEventSubscriber.hpp>
#include <libupnp-tools/UPnPNotificationServer.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>

#include <ncurses.h>

using namespace std;
using namespace UTIL;
using namespace UPNP;
using namespace HTTP;
using namespace XML;

class Win {
private:
	int _h;
	int _w;
	int _y;
	int _x;
	WINDOW * _win;
	vector<string> _contents;
	size_t _scrolly;
	string _title;
public:
	Win(int h, int w, int y, int x) : _h(h), _w(w), _y(y), _x(x), _win(NULL), _scrolly(0) {
		create();
	}
	virtual ~Win() {
		destroy();
	}
	void create() {
		create(_h, _w, _y, _x);
	}
	void create(int h, int w, int y, int x) {
		destroy();
		set(h, w, y, x);
		_win = newwin(h, w, y, x);
		box(_win, 0, 0);
		wborder(_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
		wrefresh(_win);
	}
	void destroy() {
		if (_win == NULL) {
			return;
		}
		wborder(_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
		draw();
		delwin(_win);
		_win = NULL;
	}
	void setBorder(char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8) {
		wborder(_win, c1, c2, c3, c4, c5, c6, c7, c8);
		wrefresh(_win);
	}
	void drawTitle() {
		int padsize = _w - (int)_title.size() - 1;
		wprintw(_win, "%s%s\n", _title.c_str(), (padsize > 0 ? string(padsize, '-').c_str() : ""));
	}
	void draw() {
		wclear(_win);
		size_t offset = 0;
		if (!_title.empty()) {
			drawTitle();
			offset = 1;
		}
		for (size_t i = _scrolly; i < _contents.size() && i < _scrolly + (_h - offset); i++) {
			wprintw(_win, "%s", _contents[i].substr(0, _w).c_str());
		}
		wrefresh(_win);
	}
	int & width() {
		return _w;
	}
	int & height() {
		return _h;
	}
	int & x() {
		return _x;
	}
	int & y() {
		return _y;
	}
	void set(int h, int w, int y, int x) {
		_h = h;
		_w = w;
		_y = y;
		_x = x;
	}
	size_t & scrolly() {
		return _scrolly;
	}
	void adjustScroll() {
		size_t offset = _title.empty() ? 0 : 1;
		if (_contents.size() > _scrolly + (_h - offset)) {
			_scrolly = _contents.size() - (_h - offset);
		}
	}
	void appendLine(const string & line) {
		_contents.push_back(line + "\n");
	}
	void appendLines(const vector<string> & lines) {
		for (vector<string>::const_iterator iter = lines.begin(); iter != lines.end(); iter++) {
			_contents.push_back(*iter + "\n");
		}
	}
	string & title() {
		return _title;
	}
	void clearContents() {
		_contents.clear();
	}
};


class Tui {
private:
	string _toast;
	Win * win_device_list;
	Win * win_status;
	Win * win_toast;
public:
	Tui() : win_toast(NULL) {}
	virtual ~Tui() {}
private:
	Tui(const Tui & t);
	Tui & operator=(const Tui & t);
public:
	void setNoecho() {
		noecho();
	}
	void setScroll(bool enable) {
		scrollok(stdscr, (enable ? TRUE : FALSE));
	}
	int getScreenWidth() {
		int row, col;
		getmaxyx(stdscr, row, col);
		(void)row;
		return col;
	}
	int getScreenHeight() {
		int row, col;
		getmaxyx(stdscr, row, col);
		(void)col;
		return row;
	}
	void getScreenSize(int * width, int * height) {
		int row, col;
		getmaxyx(stdscr, row, col);
		*width = col;
		*height = row;
	}
	void start() {

		int left_width = 40;
		
		initscr();
		refresh();
		win_toast = new Win(3, getScreenWidth(), getScreenHeight() - 3, 0);
		win_status = new Win(getScreenHeight() - 3, getScreenWidth() - left_width, 0, left_width);
		win_device_list = new Win(getScreenHeight() - 3, left_width, 0, 0);
		win_toast->title() = " *-* Message *-* ";
		win_status->title() = " *-* Status *-* ";
		printStatus("hello status");
		win_device_list->title() = " *-* Devices *-* ";
		draw();
	}
	void stop() {
		delete win_toast;
		delete win_status;
		delete win_device_list;
		endwin();
	}
	void onResize() {

		int left_width = 40;
		
		clear();
		refresh();
		win_toast->create(3, getScreenWidth(), getScreenHeight() - 3, 0);
		win_status->create(getScreenHeight() - 3, getScreenWidth() - left_width, 0, left_width);
		win_device_list->create(getScreenHeight() - 3, left_width, 0, 0);
		draw();
	}
	void toast(const string & msg) {
		win_toast->appendLine(msg);
		win_toast->adjustScroll();
		draw();
	}
	vector<string> parseLines(const string & msg) {
		vector<string> lines;
		size_t s = 0;
		size_t f = msg.find("\n");
		while (f != string::npos) {
			lines.push_back(msg.substr(s, f - s - 1));
			s = f + 1;
			f = msg.find("\n", s);
		}
		if (s < msg.size()) {
			lines.push_back(msg.substr(s));
		}
		return lines;
	}
	void printStatus(const string & msg) {
		win_status->appendLines(parseLines(msg));
	}
	void draw() {
		win_toast->draw();
		win_status->draw();
		win_device_list->draw();
	}
	void printDeviceList(UPnPSessionManager & sessionManager) {
		vector<string> fns;
		vector<string> lst = sessionManager.getUdnS();
		for (vector<string>::iterator iter = lst.begin(); iter != lst.end(); iter++) {
			AutoRef<UPnPSession> session = sessionManager[*iter];
			if (session->completed()) {
				string fn = session->getRootDevice()->getFriendlyName();
				fns.push_back(fn);
			}
		}
		win_device_list->clearContents();
		win_device_list->appendLines(fns);
		win_device_list->title() = " *-* " + Text::toString(fns.size()) + " Devices *-* ";
	}
};


class MyDeviceListener : public DeviceAddRemoveListener {
private:
	Tui & tui;
	UPnPControlPoint & cp;
public:
    MyDeviceListener(Tui & tui, UPnPControlPoint & cp) : tui(tui), cp(cp) {}
    virtual ~MyDeviceListener() {}

	virtual void onDeviceAdd(AutoRef<UPnPDevice> device) {
		tui.toast(" ** Added: " + device->getFriendlyName());
		tui.printDeviceList(cp.sessionManager());
	}

	virtual void onDeviceRemove(AutoRef<UPnPDevice> device) {
		tui.toast(" ** Removed: " + device->getFriendlyName());
		tui.printDeviceList(cp.sessionManager());
	}
};

string readline() {
	FileStream reader(stdin);
	return reader.readline();
}

int main(int argc, char *args[]) {

	bool done = false;

	UPnPControlPointConfig config(9998);
	UPnPControlPoint cp(config);

	Tui tui;
	tui.start();
	tui.setNoecho();

	cp.setDeviceAddRemoveListener(AutoRef<DeviceAddRemoveListener>(new MyDeviceListener(tui, cp)));
	cp.startAsync();

	while (!done) {
		int ch = getch();

		switch (ch) {
		case KEY_RESIZE:
			tui.onResize();
			break;
		case 'q':
			done = true;
			break;
		case 's':
			tui.toast(" ** searching **");
			cp.sendMsearchAndWait("upnp:rootdevice", 3);
			tui.toast(" ** searching : done **");
			break;
		case 'r':
			tui.printDeviceList(cp.sessionManager());
			tui.draw();
			break;
		default:
			break;
		}
	}

	cp.stop();

	tui.stop();

    return 0;
}
