#include "XmlEncoderDecoder.hpp"
#include <liboslayer/Text.hpp>

namespace XML {
    
    using namespace std;
    using namespace UTIL;
    
    XmlEncoder::XmlEncoder() {
    }
    XmlEncoder::~XmlEncoder() {
    }
    string XmlEncoder::encode(const string & text) {
        string encoded = text;
        encoded = Text::replaceAll(encoded, "&", "&amp;"); // & must be replaced at first
        encoded = Text::replaceAll(encoded, "<", "&lt;");
        encoded = Text::replaceAll(encoded, ">", "&gt;");
        return encoded;
    }

    XmlDecoder::XmlDecoder() {
    }
    XmlDecoder::~XmlDecoder() {
    }
    string XmlDecoder::decode(const string & text) {
        string decoded = text;
        decoded = Text::replaceAll(decoded, "&lt;", "<");
        decoded = Text::replaceAll(decoded, "&gt;", ">");
        decoded = Text::replaceAll(decoded, "&amp;", "&"); // & must be replaced at last
        return decoded;
    }
}