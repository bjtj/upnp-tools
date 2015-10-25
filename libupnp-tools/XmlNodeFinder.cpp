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
		TagNameCondition condition(tagName, 1);
		vector<XmlNode> nodes = collect(condition);
		return nodes.size() > 0 ? nodes[0] : XmlNode();
	}
	
	vector<XmlNode> XmlNodeFinder::getAllNodesByTagName(const string & tagName) {
		TagNameCondition cond(tagName, -1);
		return collect(cond);
	}
	
	void XmlNodeFinder::iterate(IteratorCallback<XmlNode> & callback) {
		iterate_r(doc.getRootNode(), callback);
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
	
	vector<XmlNode> XmlNodeFinder::collect(Condition<XmlNode> & condition) {
		CollectIterator iter(condition);
		iterate(iter);
		return iter.getResult();
	}


	string XmlNodeFinder::getContentByTagName(const string & tagName) {
		XmlNode node = getNodeByTagName(tagName);
		return node.size() > 0 ? node[0].getData() : "";
	}
}
