#ifndef __XML_NODE_FINDER_HPP__
#define __XML_NODE_FINDER_HPP__

#include <string>
#include <vector>
#include "XmlDocument.hpp"

namespace XML {

    /**
     * @brief iteration condition
     */
	template <typename T>
	class Condition {
	private:
		std::string rule;
		
	public:
		Condition(const std::string & rule);
		virtual ~Condition();

		virtual bool test(const T & item) const;
		std::string & getRule();
		virtual bool wantFinish();
	};

    /**
     * @brief iterator callback
     */
	template <typename T>
	class IteratorCallback {
	private:
	public:
		IteratorCallback() {}
		virtual ~IteratorCallback() {}
		virtual void onItem(const T & item) = 0;
		virtual bool wantFinish() {return false;}
	};


    /**
     * @brief xml node finder
     */
	class XmlNodeFinder {
	private:
		const XmlDocument & doc;
		
	public:
		XmlNodeFinder(const XmlDocument & doc);
		virtual ~XmlNodeFinder();

		XmlNode getNodeByTagName(const std::string & tagName);
		std::vector<XmlNode> getAllNodesByTagName(const std::string & tagName);
		void iterate(IteratorCallback<XmlNode> & callback);
		std::vector<XmlNode> collect(Condition<XmlNode> & condition);
		std::string getContentByTagName(const std::string & tagName);
		std::vector<std::string> getAllContentsByTagName(const std::string & tagName);

	public:
		static XmlNode getNodeByTagName(const XmlNode & node, const std::string & tagName, int maxDepth = -1);
		static std::vector<XmlNode> getAllNodesByTagName(const XmlNode & node, const std::string & tagName, int maxDepth = -1);
		static void iterate(const XmlNode & node, IteratorCallback<XmlNode> & callback, int maxDepth = -1);
		static std::vector<XmlNode> collect(const XmlNode & node, Condition<XmlNode> & condition, int maxDepth = -1);
		static std::string getContentByTagName(const XmlNode & node, const std::string & tagName, int maxDepth = -1);
		static std::vector<std::string> getAllContentsByTagName(const XmlNode & node, const std::string & tagName, int maxDepth = -1);

	private:
		static void iterate_r(const XmlNode & node, IteratorCallback<XmlNode> & callback, int depth, int maxDepth = -1);
	};

}

#endif
