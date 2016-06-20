#ifndef __XML_UTILS_HPP__
#define __XML_UTILS_HPP__

#include <string>
#include <liboslayer/XmlParser.hpp>
#include <liboslayer/StringElements.hpp>

namespace UPNP {
	class XmlUtils {
	private:
	public:
		XmlUtils() {}
		virtual ~XmlUtils() {}

		static UTIL::NameValue toNameValue(XML::XmlNode * node) {

			if (!node) {
				throw OS::Exception("node null", -1, 0);
			}
			
			UTIL::NameValue nv;
			nv.name() = node->tagName();
			nv.value() = (node->getFirstChild() ? node->getFirstChild()->text() : "");
			return nv;
		}

		static bool testNameValueXmlNode(XML::XmlNode * node) {
			return node && node->isElement() && node->childrenCount() == 1 &&
				node->getFirstChild() && node->getFirstChild()->isText();
		}

		static std::string toNameValueTag(const UTIL::NameValue & nv) {
			std::string tag = XML::XmlEncoder::encode(nv.const_name());
			std::string encodedContent = XML::XmlEncoder::encode(nv.const_value());
			return ("<" + tag + ">" + encodedContent + "</" + tag + ">");
		}

		static std::string toNameValueTag(const std::string & name, const std::string & value) {
			std::string tag = XML::XmlEncoder::encode(name);
			std::string encodedContent = XML::XmlEncoder::encode(value);
			return ("<" + tag + ">" + encodedContent + "</" + tag + ">");
		}
	};
}

#endif
