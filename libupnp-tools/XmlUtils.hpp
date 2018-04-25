#ifndef __XML_UTILS_HPP__
#define __XML_UTILS_HPP__

#include <string>
#include <liboslayer/XmlParser.hpp>
#include <liboslayer/StringElements.hpp>

namespace upnp {
	
	class XmlUtils {
	private:
	public:
		XmlUtils() {}
		virtual ~XmlUtils() {}

		static osl::KeyValue toKeyValue(osl::AutoRef<osl::XmlNode> node) {

			if (node.nil()) {
				throw osl::Exception("node null", -1, 0);
			}
			
			osl::KeyValue nv;
			nv.key() = node->tagName();
			nv.value() = (node->getFirstChild().nil() == false ? node->getFirstChild()->text() : "");
			return nv;
		}

		static bool testKeyValueXmlNode(osl::AutoRef<osl::XmlNode> node) {
			return (node.nil() == false && node->isElement() && node->childrenCount() == 1 &&
					node->getFirstChild().nil() == false && node->getFirstChild()->isText());
		}

		static std::string toKeyValueTag(const osl::KeyValue & nv) {
			std::string tag = osl::XmlEncoder::encode(nv.key());
			std::string encodedContent = osl::XmlEncoder::encode(nv.value());
			return ("<" + tag + ">" + encodedContent + "</" + tag + ">");
		}

		static std::string toKeyValueTag(const std::string & name, const std::string & value) {
			std::string tag = osl::XmlEncoder::encode(name);
			std::string encodedContent = osl::XmlEncoder::encode(value);
			return ("<" + tag + ">" + encodedContent + "</" + tag + ">");
		}
	};
}

#endif
