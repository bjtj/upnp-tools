#ifndef __XML_DOM_PARSER_HPP__
#define __XML_DOM_PARSER_HPP__

#include "XmlDocument.hpp"
#include "StringReader.hpp"

#include <vector>
#include <string>

namespace XML {
    
    /**
	 * @brief xml parse cursor
	 */
    class XmlParseCursor {
    private:
        XmlNode rootNode;
        XmlNode * cursor;
    public:
        XmlParseCursor();
        virtual ~XmlParseCursor();
        
        void enter(XmlNode & node);
        void leave();
        void append(XmlNode & node);
        
        XmlNode & getRootNode();
    };

	class XmlTokenizer
	{
	public:
		XmlTokenizer();
		virtual ~XmlTokenizer();

		static std::string getNamespace(std::string & symbol);
		static std::string getName(std::string & symbol);
	};


	/**
	 * @brief xml string reader
	 */
	class XmlStringReader : public StringReader {
	private:
		const char * specials;
	public:
		XmlStringReader(const std::string & data);
		virtual ~XmlStringReader();

		bool isBlocker(char ch);
		bool isSpecial(char ch);

		std::string quote(const std::string & str);

		std::string readAttribute(XmlAttribute & attribute);
		std::string readSymbol();
		std::string readSpecial();
		std::string readQuotedString();
		std::string readQuotedStringWithoutQuote();
		std::string readTag(XmlNode & node);
        std::string readText();
        std::string readPrologue();
	};


	/**
	 * @brief xml dom parser
	 */
	class XmlDomParser {
	private:
        XmlParseCursor cursor;
		
	public:
		XmlDomParser();
		virtual ~XmlDomParser();

		bool isStartTag(std::string & tag);
		bool isEndTag(std::string & tag);
		bool isSingleTag(std::string & tag);
		
		XmlDocument parse(const std::string & data);
	};
}

#endif
