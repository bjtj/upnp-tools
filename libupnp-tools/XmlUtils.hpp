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

		static UTIL::KeyValue toKeyValue(OS::AutoRef<XML::XmlNode> node) {

			if (node.nil()) {
				throw OS::Exception("node null", -1, 0);
			}
			
			UTIL::KeyValue nv;
			nv.key() = node->tagName();
			nv.value() = (node->getFirstChild().nil() == false ? node->getFirstChild()->text() : "");
			return nv;
		}

		static bool testKeyValueXmlNode(OS::AutoRef<XML::XmlNode> node) {
			return (node.nil() == false && node->isElement() && node->childrenCount() == 1 &&
					node->getFirstChild().nil() == false && node->getFirstChild()->isText());
		}

		static std::string toKeyValueTag(const UTIL::KeyValue & nv) {
			std::string tag = XML::XmlEncoder::encode(nv.key());
			std::string encodedContent = XML::XmlEncoder::encode(nv.value());
			return ("<" + tag + ">" + encodedContent + "</" + tag + ">");
		}

		static std::string toKeyValueTag(const std::string & name, const std::string & value) {
			std::string tag = XML::XmlEncoder::encode(name);
			std::string encodedContent = XML::XmlEncoder::encode(value);
			return ("<" + tag + ">" + encodedContent + "</" + tag + ">");
		}
	};
}

#endif
