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
	bool Condition<T>::test(T & item) {
		return false;
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

		virtual void onItem(XmlNode & node) {
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

		virtual bool test(XmlNode & item) {
			bool ret = (!getRule().compare(item.getTagName()));
			if (ret) {
				count++;
			}
			return ret;
		}
		virtual bool wantFinish() {
			return (max >= 0 && count >= max);
		}
	};


	XmlNodeFinder::XmlNodeFinder(XmlDocument & doc) : doc(doc){
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



	XmlNode XmlNodeFinder::getNodeByTagName(XmlNode & node, const string & tagName) {
		TagNameCondition condition(tagName, 1);
		vector<XmlNode> nodes = XmlNodeFinder::collect(node, condition);
		return nodes.size() > 0 ? nodes[0] : XmlNode();
	}

	vector<XmlNode> XmlNodeFinder::getAllNodesByTagName(XmlNode & node, const string & tagName) {
		TagNameCondition cond(tagName, -1);
		return XmlNodeFinder::collect(node, cond);
	}

	void XmlNodeFinder::iterate(XmlNode & node, IteratorCallback<XmlNode> & callback) {
		iterate_r(node, callback);
	}

	vector<XmlNode> XmlNodeFinder::collect(XmlNode & node, Condition<XmlNode> & condition) {
		CollectIterator iter(condition);
		XmlNodeFinder::iterate(node, iter);
		return iter.getResult();
	}

	string XmlNodeFinder::getContentByTagName(XmlNode & node, const string & tagName) {
		XmlNode found = XmlNodeFinder::getNodeByTagName(node, tagName);
		return found.getFirstContent();
	}

	vector<string> getAllContentsByTagName(XmlNode & node, const string & tagName) {
		vector<string> ret;
		vector<XmlNode> nodes = XmlNodeFinder::getAllNodesByTagName(node, tagName);
		for (size_t i = 0; i < nodes.size(); i++) {
			XmlNode & found = nodes[i];
			ret.push_back(found.getFirstContent());
		}
		return ret;
	}

	void XmlNodeFinder::iterate_r(XmlNode & node, IteratorCallback<XmlNode> & callback) {
		if (callback.wantFinish()) {
			return;
		}
		callback.onItem(node);
		for (size_t i = 0; i < node.getChildren().size(); i++) {
			iterate_r(node.getNode(i), callback);
		}
	}
}
