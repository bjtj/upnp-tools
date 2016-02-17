#ifndef __XML_UTILS_HPP__
#define __XML_UTILS_HPP__

#include <string>
#include <liboslayer/XmlParser.hpp>
#include <liboslayer/StringElement.hpp>

namespace UPNP {
	class XmlUtils {
	private:
	public:
		XmlUtils() {}
		virtual ~XmlUtils() {}

		static UTIL::NameValue toNameValue(XML::XmlNode * node) {
			UTIL::NameValue nv;
			std::string name = node->tagName();
			std::string value = node->getFirstChild()->text();
			nv.getName() = name;
			nv.getValue() = value;
			return nv;
		}

		static bool testNameValueXmlNode(XML::XmlNode * node) {
			return node && node->isElement() && node->childrenCount() == 1 &&
				node->getFirstChild() && node->getFirstChild()->isText();
		}
	};

}

#endif
