#ifndef __STRING_READER_HPP__
#define __STRING_READER_HPP__

#include <string>

namespace XML {
	
	class StringReader {
	private:
		const char * spaces;
		const std::string & data;
		size_t pos;
		
	public:
		StringReader(const std::string & data);
		virtual ~StringReader();

		void setSpaces(const char * spaces);
		std::string read();
		std::string read(size_t count);
		std::string readWithoutMove(size_t count);
		std::string readSpaces();
		size_t moveForward(size_t forward);
		bool contains(char ch, const char * list);
		void skipSpaces();
		bool endOfString();
		const std::string & getData();
		bool isSpace(char ch);
	};
}

#endif
