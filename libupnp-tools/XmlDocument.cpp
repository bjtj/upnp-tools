#include "XmlDocument.hpp"
#include "macros.hpp"
#include <liboslayer/os.hpp>

namespace XML {

    using namespace OS;
	using namespace std;

	static XmlAttribute EMPTY_ATTRIBUTE;

    XmlNamespace::XmlNamespace() {
        
    }
    
    XmlNamespace::~XmlNamespace() {
        
    }
    
    string XmlNamespace::getAlias() {
        return alias;
    }
    string XmlNamespace::getUri() {
        return uri;
    }
    
	XmlAttribute::XmlAttribute() {
	}

	XmlAttribute::XmlAttribute(string name, string value) : name(name), value(value) {
	}

	XmlAttribute::~XmlAttribute() {
	}

    void XmlAttribute::setNamespace(const string & ns) {
        this->ns = ns;
    }
	void XmlAttribute::setName(const string & name) {
		this->name = name;
	}
	void XmlAttribute::setValue(const string & value) {
		this->value = value;
	}
    string XmlAttribute::getNamespace() const {
        return ns;
    }
	string XmlAttribute::getName() const {
		return name;
	}
	string XmlAttribute::getValue() const {
		return value;
	}

    
    XmlNode::XmlNode() : parent(NULL) {
    }
    
    XmlNode::~XmlNode() {
    }

	const string & XmlNode::getText() const {
        return text;
    }
	
    void XmlNode::setText(const string & text) {
        this->text = text;
    }

	const string & XmlNode::getNamespace() const {
		return ns;
	}
	
	void XmlNode::setNamespace(const string & ns) {
		this->ns = ns;
	}

    const string & XmlNode::getTagName() const {
		return tagName;
	}

	void XmlNode::setTagName(const string & tagName) {
		this->tagName = tagName;
	}
	
    void XmlNode::setParent(XmlNode * parent) {
        this->parent = parent;
    }
    
    XmlNode * XmlNode::getParent() {
        return parent;
    }
    
    void XmlNode::addNode(const XmlNode & node) {
        children.push_back(node);
        (*children.rbegin()).setParent(this);
    }
    
    const XmlNode & XmlNode::getNode(size_t index) const {
        return children[index];
    }
    
    const vector<XmlNode> & XmlNode::getChildren() const {
		return children;
	}

	bool XmlNode::empty() {
		return text.empty() && tagName.empty();
	}

	size_t XmlNode::size() {
		return children.size();
	}

	void XmlNode::setAttribute(const string & name, const string & value) {
		for (size_t i = 0; i < attributes.size(); i++) {
			XmlAttribute & attr = attributes[i];
			if (!attr.getName().compare(name)) {
				attr.setValue(value);
				return;
			}
		}

		attributes.push_back(XmlAttribute(name, value));
	}
	
	void XmlNode::setAttribute(const XmlAttribute & attribute) {
		for (size_t i = 0; i < attributes.size(); i++) {
			XmlAttribute & attr = attributes[i];
			if (!attr.getName().compare(attribute.getName())) {
				attr.setValue(attribute.getValue());
				return;
			}
		}

		attributes.push_back(attribute);
	}
	
	const XmlAttribute & XmlNode::getAttribute(const string & name) const {
		for (size_t i = 0; i < attributes.size(); i++) {
			const XmlAttribute & attr = attributes[i];
			if (!attr.getName().compare(name)) {
				return attr;
			}
		}
		return EMPTY_ATTRIBUTE;
	}

	string XmlNode::getAttributeValue(const string & name) const {
		return getAttribute(name).getValue();
	}

	const vector<XmlAttribute> & XmlNode::getAttributes() const {
		return attributes;
	}

	string XmlNode::getFirstContent() const {
		return children.size() > 0 ? children[0].getText() : "";
	}

	bool XmlNode::isTextNode() const {
		return !text.empty();
	}
	
	bool XmlNode::isElementNode() const {
		return !tagName.empty();
	}

	int XmlNode::getChildrenElementNodeCount() const {
		int cnt = 0;
		LOOP_VEC(children, i) {
			const XmlNode & node = children[i];
			if (node.isElementNode()) {
				cnt++;
			}
		}
		return cnt;
	}
    
    XmlNode & XmlNode::getFirstElement() {
        LOOP_VEC(children, i) {
            if (children[i].isElementNode()) {
                return children[i];
            }
        }
        throw Exception("no first element", -1, 0);
    }
    
    const XmlNode & XmlNode::getFirstElement() const {
        LOOP_VEC(children, i) {
            if (children[i].isElementNode()) {
                return children[i];
            }
        }
        throw Exception("no first element", -1, 0);
    }
    
    XmlNode & XmlNode::getLastNode() {
        return *children.rbegin();
    }
	
	const XmlNode& XmlNode::operator[](size_t index) const {
		return getNode(index);
	}

	
    XmlDocument::XmlDocument() {
	}
    
	XmlDocument::~XmlDocument() {
	}
    
    void XmlDocument::addNamespacePhase(string & nsPhrase) {
        
    }
    
    XmlNamespace XmlDocument::getNamespaceWithAlias(const string & alias) {
        for (size_t i = 0; i < xmlNamespaces.size(); i++) {
            XmlNamespace ns = xmlNamespaces[i];
            if (!ns.getAlias().compare(alias)) {
                return ns;
            }
        }
        return XmlNamespace();
    }
    
    const XmlNode & XmlDocument::getRootNode() const {
        return rootNode;
    }

	void XmlDocument::setRootNode(XmlNode & rootNode) {
		this->rootNode = rootNode;
	}

    void XmlDocument::setPrologue(const string & prologue) {
        this->prologue = prologue;
    }

	string & XmlDocument::getPrologue() {
		return prologue;
	}
	const string & XmlDocument::getPrologue() const {
		return prologue;
	}
}
