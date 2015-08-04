#include <iostream>
#include <string>
#include <map>
#include "Text.hpp"

// #include "UPnPControlPoint.hpp"

using namespace std;
using namespace UTIL;


/**
 * proc
 */
class Proc {
private:
public:
    Proc() {}
    virtual ~Proc() {}

	virtual void proc(vector<string> & toks) = 0;
};

/**
 * repl
 */
class Repl : public Proc {
private:
	bool done;
	map<string, Proc*> procs;
public:
    Repl();
    virtual ~Repl();

	virtual void loop();
	virtual void proc(vector<string> & toks);
	void finish();

	void addProc(string name, Proc* proc);
	void removeProc(string name);
	Proc * getProc(string name);
};

Repl::Repl() {
}

Repl::~Repl() {
}

void Repl::loop() {
	while (!done) {
		string line;
		std::getline(std::cin, line);
		vector<string> toks = Text::split(line, " ");
		proc(toks);
	}
}

void Repl::proc(vector<string> & toks) {
	if (toks.size() > 0) {
		Proc * p = getProc(toks[0]);
		if (p) {
			p->proc(toks);
		}
	}
}

void Repl::finish() {
	done = true;
}

void Repl::addProc(string name, Proc* proc) {
	procs[name] = proc;
}
void Repl::removeProc(string name) {
	procs.erase(name);
}
Proc * Repl::getProc(string name) {
	return procs[name];
}



/**
 * main
 */
int main(int argc, char *args[]) {

	Repl repl;
	repl.loop();

    return 0;
}
