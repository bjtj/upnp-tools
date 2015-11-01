#include "XmlNodeFinder.hpp"

namespace XML {

	using namespace std;

	template<typename T>
	Condition<T>::Condition(const std::string & rule) : rule(rule) {
	}

	template<typename T>
	Condition<T>::~Condition() {
	}

	template<typename T>
	string & Condition<T>::getRule() {
		return rule;
	}

	template<typename T>
	bool Condition<T>::wantFinish() {
		return false;
	}


	class CollectIterator : public IteratorCallback<XmlNode> {
	private:
		Condition<XmlNode> & condition;
		vector<XmlNode> result;
		
	public:
		CollectIterator(Condition<XmlNode> & condition) : condition(condition) {}
		virtual ~CollectIterator() {}

		virtual void onItem(const XmlNode & node) {
			if (condition.test(node)) {
				result.push_back(node);
			}
		}

		virtual bool wantFinish() {
			return condition.wantFinish();
		}

		vector<XmlNode> & getResult() {
			return result;
		}
	};

	

	class TagNameCondition : public Condition<XmlNode> {
	private:
		int count;
		int max;
		
	public:
		TagNameCondition(const string & rule, int max) : Condition<XmlNode>(rule), count(0), max(max) {
		}
		virtual ~TagNameCondition() {}

		virtual bool test(const XmlNode & item) {
			string rule = getRule();
			bool ret = (!rule.compare(item.getTagName()));
			if (ret) {
				count++;
			}
			return ret;
		}
		virtual bool wantFinish() {
			return (max >= 0 && count >= max);
		}
	};


	XmlNodeFinder::XmlNodeFinder(const XmlDocument & doc) : doc(doc){
	}
	
	XmlNodeFinder::~XmlNodeFinder() {
	}

	XmlNode XmlNodeFinder::getNodeByTagName(const string & tagName) {
		return XmlNodeFinder::getNodeByTagName(doc.getRootNode(), tagName);
	}
	
	vector<XmlNode> XmlNodeFinder::getAllNodesByTagName(const string & tagName) {
		return XmlNodeFinder::getAllNodesByTagName(doc.getRootNode(), tagName);
	}
	
	void XmlNodeFinder::iterate(IteratorCallback<XmlNode> & callback) {
		XmlNodeFinder::iterate(doc.getRootNode(), callback);
	}
	
	vector<XmlNode> XmlNodeFinder::collect(Condition<XmlNode> & condition) {
		return XmlNodeFinder::collect(doc.getRootNode(), condition);
	}
	
	string XmlNodeFinder::getContentByTagName(const string & tagName) {
		return XmlNodeFinder::getContentByTagName(doc.getRootNode(), tagName);
	}


	XmlNode XmlNodeFinder::getNodeByTagName(const XmlNode & node, const string & tagName, int maxDepth) {
		TagNameCondition condition(tagName, 1);
		vector<XmlNode> nodes = XmlNodeFinder::collect(node, condition, maxDepth);
		return nodes.size() > 0 ? nodes[0] : XmlNode();
	}

	vector<XmlNode> XmlNodeFinder::getAllNodesByTagName(const XmlNode & node, const string & tagName, int maxDepth) {
		TagNameCondition cond(tagName, -1);
		return XmlNodeFinder::collect(node, cond, maxDepth);
	}

	void XmlNodeFinder::iterate(const XmlNode & node, IteratorCallback<XmlNode> & callback, int maxDepth) {
		iterate_r(node, callback, 0, maxDepth);
	}

	vector<XmlNode> XmlNodeFinder::collect(const XmlNode & node, Condition<XmlNode> & condition, int maxDepth) {
		CollectIterator iter(condition);
		XmlNodeFinder::iterate(node, iter, maxDepth);
		return iter.getResult();
	}

	string XmlNodeFinder::getContentByTagName(const XmlNode & node, const string & tagName, int maxDepth) {
		XmlNode found = XmlNodeFinder::getNodeByTagName(node, tagName, maxDepth);
		return found.getFirstContent();
	}

	vector<string> XmlNodeFinder::getAllContentsByTagName(const XmlNode & node, const string & tagName, int maxDepth) {
		vector<string> ret;
		vector<XmlNode> nodes = XmlNodeFinder::getAllNodesByTagName(node, tagName, maxDepth);
		for (size_t i = 0; i < nodes.size(); i++) {
			XmlNode & found = nodes[i];
			ret.push_back(found.getFirstContent());
		}
		return ret;
	}

	void XmlNodeFinder::iterate_r(const XmlNode & node, IteratorCallback<XmlNode> & callback, int depth, int maxDepth) {
		if (callback.wantFinish()) {
			return;
		}
		callback.onItem(node);
		if (maxDepth >= 0 && depth >= maxDepth) {
			return;
		}
		depth++;
		for (size_t i = 0; i < node.getChildren().size(); i++) {
			iterate_r(node[i], callback, depth, maxDepth);
		}
	}
}
