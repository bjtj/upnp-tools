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
	};

}

#endif
