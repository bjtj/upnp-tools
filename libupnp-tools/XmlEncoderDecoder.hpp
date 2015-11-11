#ifndef __XML_ENCODER_DECODER_HPP__
#define __XML_ENCODER_DECODER_HPP__

#include <string>

namespace XML {
    class XmlEncoder {
    private:
    public:
        XmlEncoder();
        virtual ~XmlEncoder();
        
        static std::string encode(const std::string & text);
    };
    
    class XmlDecoder {
    private:
    public:
        XmlDecoder();
        virtual ~XmlDecoder();
        
        static std::string decode(const std::string & text);
    };
}

#endif
