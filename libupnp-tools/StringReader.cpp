#include "StringReader.hpp"

namespace XML {

	using namespace std;
	
	StringReader::StringReader(string & data) : spaces(" \t\r\n"), data(data), pos(0) {
	}
	
	StringReader::~StringReader() {
	}

	void StringReader::setSpaces(const char * spaces) {
		this->spaces = spaces;
	}

	string StringReader::read() {
		string ret;
		if (endOfString()) {
			return "";
		}

		ret = data.substr(pos);
		pos = data.length();
		return ret;
	}
	
	string StringReader::read(size_t count) {
		size_t e = pos + count;
		if (e > data.length()) {
			e = data.length();
		}
		if (pos == e) {
			return "";
		}
		string ret = data.substr(pos, e - pos);
		pos = e;
		return ret;
	}

	string StringReader::readWithoutMove(size_t count) {
		size_t e = pos + count;
		if (e > data.length()) {
			e = data.length();
		}
		if (pos == e) {
			return "";
		}
		string ret = data.substr(pos, e - pos);
		return ret;
	}

	string StringReader::readSpaces() {

		string ret;
		if (endOfString()) {
			return "";
		}

		size_t f = data.find_first_not_of(spaces, pos);
		if (f == string::npos) {
			return read();
		}

		if (f == pos) {
			return "";
		}

		ret = data.substr(pos, f - pos);
		pos = f;
		return ret;
	}

	size_t StringReader::moveForward(size_t forward) {
		if (pos + forward < data.length()) {
			pos += forward;
			return forward;
		} else {
			size_t ret = data.length() - (pos > data.length() ? data.length() : pos);
			pos = data.length();
			return ret;
		}
	}
	
	bool StringReader::contains(char ch, const char * list) {
		const char * p = list;
		while (*p) {
			if (ch == *(p++)) {
				return true;
			}
		}
		return false;
	}

	void StringReader::skipSpaces() {
		while (!endOfString() && contains(readWithoutMove(1)[0], spaces)) {
			read(1);
		}
	}

	bool StringReader::endOfString() {
		return pos >= data.length();
	}

	string & StringReader::getData() {
		return data;
	}

	bool StringReader::isSpace(char ch) {
		return contains(ch, spaces);
	}
}
