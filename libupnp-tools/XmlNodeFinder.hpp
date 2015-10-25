#ifndef __XML_NODE_FINDER_HPP__
#define __XML_NODE_FINDER_HPP__

#include <string>
#include <vector>
#include "XmlDocument.hpp"

namespace XML {

	template <typename T>
	class Condition {
	private:
		std::string rule;
		
	public:
		Condition(const std::string & rule);
		virtual ~Condition();

		virtual bool test(T & item);
		std::string & getRule();
		virtual bool wantFinish();
	};

	template <typename T>
	class IteratorCallback {
	private:
	public:
		IteratorCallback() {}
		virtual ~IteratorCallback() {}
		virtual void onItem(T & item) = 0;
		virtual bool wantFinish() {return false;}
	};


	class XmlNodeFinder {
	private:
		XmlDocument & doc;
		
	public:
		XmlNodeFinder(XmlDocument & doc);
		virtual ~XmlNodeFinder();

		XmlNode getNodeByTagName(const std::string & tagName);
		std::vector<XmlNode> getAllNodesByTagName(const std::string & tagName);
		void iterate(IteratorCallback<XmlNode> & callback);
		std::vector<XmlNode> collect(Condition<XmlNode> & condition);

		std::string getContentByTagName(const std::string & tagName);

	private:
		void iterate_r(XmlNode & node, IteratorCallback<XmlNode> & callback);
	};

}

#endif