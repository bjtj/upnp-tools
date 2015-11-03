#include <liboslayer/Text.hpp>

#include "XmlDomParser.hpp"

#include <iostream>

namespace XML {

	using namespace std;
	using namespace UTIL;
    
    XmlParseCursor::XmlParseCursor() : cursor(NULL) {
    }
    XmlParseCursor::~XmlParseCursor() {
    }
    
    void XmlParseCursor::enter(XmlNode & node) {
        if (cursor) {
            cursor = cursor->addNode(node);
        } else {
			rootNode = node;
			cursor = &rootNode;
		}
    }
    void XmlParseCursor::leave() {
        if (cursor) {
            cursor = cursor->getParent();
        }
    }
    void XmlParseCursor::append(XmlNode & node) {
        if (cursor) {
            cursor->addNode(node);
        }
    }
    
    XmlNode & XmlParseCursor::getRootNode() {
        return rootNode;
    }


	XmlTokenizer::XmlTokenizer() {
	}
	
	XmlTokenizer::~XmlTokenizer() {
	}

	string XmlTokenizer::getNamespace(string & symbol) {
		size_t f = symbol.find(":");
		if (f == string::npos || f == 0) {
			return "";
		}
		return symbol.substr(0, f);
	}
	
	string XmlTokenizer::getName(string & symbol) {
		size_t f = symbol.find(":");
		if (f == string::npos) {
			return symbol;
		}
		return symbol.substr(f+1);
	}



	XmlStringReader::XmlStringReader(const string & data) : StringReader(data), specials("=") {
		setSpaces(" \t\r\n");
	}

	XmlStringReader::~XmlStringReader() {
	}

	string XmlStringReader::quote(const string & str) {
		return "\"" + str + "\"";
	}

	bool XmlStringReader::isBlocker(char ch) {
		return (isSpace(ch) || isSpecial(ch) || contains(ch, "<>/"));
	}

	bool XmlStringReader::isSpecial(char ch) {
		return contains(ch, specials);
	}

	string XmlStringReader::readAttribute(XmlAttribute & attribute) {
		
		string ret;
		
		ret.append(readSpaces());
		string symbol = readSymbol();
		if (symbol.empty()) {
			return ret;
		}
        attribute.setNamespace(XmlTokenizer::getNamespace(symbol));
        attribute.setName(XmlTokenizer::getName(symbol));
		ret.append(symbol);
		ret.append(readSpaces());
		string special = readSpecial();
		if (special.empty()) {
			return ret;
		}
		
		ret.append(special);
		ret.append(readSpaces());
		string value = readQuotedStringWithoutQuote();
		ret.append(quote(value));
		attribute.setValue(value);
		
		return ret;
	}
	
	string XmlStringReader::readSymbol() {
		string symbol;
		while (!endOfString() && !isBlocker(readWithoutMove(1)[0])) {
			symbol.append(read(1));
		}
		return symbol;
	}
	
	string XmlStringReader::readSpecial() {
		string special;
		while (!endOfString() && isSpecial(readWithoutMove(1)[0])) {
			special.append(read(1));
		}
		return special;
	}
	string XmlStringReader::readQuotedString() {
		
		string text;
		string ch;
		if (readWithoutMove(1).compare("\"")) {
			return "";
		}

		text.append(read(1));
		
		while (!endOfString() && (ch = read(1)).compare("\"")) {
			text.append(ch);
			if (!ch.compare("\\")) {
				text.append(read(1));
			}
		}

		if (!endOfString()) {
			text.append(ch);
		}
		
        return text;
	}

	string XmlStringReader::readQuotedStringWithoutQuote() {
		
		string text;
		string ch;
		if (readWithoutMove(1).compare("\"")) {
			return "";
		}

		// text.append(read(1));
		
		while (!endOfString() && (ch = readWithoutMove(1)).compare("\"")) {
			text.append(read(1));
			if (!ch.compare("\\")) {
				text.append(read(1));
			}
		}

		if (!endOfString()) {
			text.append(ch);
		}
		
        return text;
	}

	string XmlStringReader::readTag(XmlNode & node) {

		string tag;
        
		readSpaces();
        
        if (endOfString() || readWithoutMove(1).compare("<")) {
            return "";
        }

		tag.append(read(1));

		if (!readWithoutMove(1).compare("/")) {
			tag.append(read(1));
		}

		tag.append(readSpaces());

		string tagName = readSymbol();
		string ns = XmlTokenizer::getNamespace(tagName);
		tagName = XmlTokenizer::getName(tagName);
		tag.append(tagName);
		node.setNamespace(ns);
		node.setTagName(tagName);

		string attr;
		XmlAttribute attribute;
		while (!(attr = readAttribute(attribute)).empty()) {
			tag.append(attr);
			node.setAttribute(attribute);
		}

		while (!endOfString() && readWithoutMove(1).compare(">")) {
			tag.append(read(1));
		}

		if (!endOfString()) {
			tag.append(read(1));
		}

		return tag;
	}
    
    string XmlStringReader::readText() {
        string text;
        while (!endOfString() && readWithoutMove(1).compare("<")) {
            text.append(read(1));
        }
        return text;
    }
    
    std::string XmlStringReader::readPrologue() {
        string ret;
        if (endOfString() || readWithoutMove(2).compare("<?")) {
            return "";
        }
        ret.append(read(2));
        while (!endOfString() && readWithoutMove(2).compare("?>")) {
            ret.append(read(1));
        }
        if (!endOfString()) {
            ret.append(read(2));
        }
        return ret;
    }

	
	
	
	
	XmlDomParser::XmlDomParser() {
	}

	XmlDomParser::~XmlDomParser() {
	}

	bool XmlDomParser::isStartTag(string & tag) {
		return Text::startsWith(tag, "<") && Text::endsWith(tag, ">") &&
			!Text::startsWith(tag, "</") && !Text::endsWith(tag, "/>");
	}
	
	bool XmlDomParser::isEndTag(string & tag) {
		return Text::startsWith(tag, "</") && Text::endsWith(tag, ">");
	}
	
	bool XmlDomParser::isSingleTag(string & tag) {
		return Text::startsWith(tag, "<") && Text::endsWith(tag, "/>");
	}

	XmlDocument XmlDomParser::parse(const string & data) {
        
        XmlDocument doc;
        XmlStringReader reader(data);
        string prologue = reader.readPrologue();
        doc.setPrologue(prologue);

		while (!reader.endOfString()) {
			XmlNode elementNode;
			string tag = reader.readTag(elementNode);
			if (isStartTag(tag)) {
				cursor.enter(elementNode);
			} else if (isEndTag(tag)) {
				cursor.leave();
			} else if (isSingleTag(tag)) {
				cursor.enter(elementNode);
				cursor.leave();
			}

			string text = reader.readText();
			XmlNode textNode;
			textNode.setData(text);
			cursor.append(textNode);
		}

		doc.setRootNode(cursor.getRootNode());
        
		return doc;
	}
	
}  // XML
