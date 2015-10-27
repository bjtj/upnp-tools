#include "XmlDocument.hpp"
#include "macros.hpp"

namespace XML {

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
    string XmlAttribute::getNamespace() {
        return ns;
    }
	string XmlAttribute::getName() {
		return name;
	}
	string XmlAttribute::getValue() {
		return value;
	}

    
    XmlNode::XmlNode() : parent(NULL) {
    }
    
    XmlNode::~XmlNode() {
    }

	string & XmlNode::getData() {
        return data;
    }
	
    void XmlNode::setData(const string & data) {
        this->data = data;
    }

	string & XmlNode::getNamespace() {
		return ns;
	}
	
	void XmlNode::setNamespace(const string & ns) {
		this->ns = ns;
	}

    string & XmlNode::getTagName() {
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
    
    XmlNode * XmlNode::addNode(XmlNode & node) {
        node.setParent(this);
        children.push_back(node);
        return &(children[children.size() - 1]);
    }
    
    XmlNode & XmlNode::getNode(size_t index) {
        return children[index];
    }
    
    vector<XmlNode> & XmlNode::getChildren() {
		return children;
	}

	bool XmlNode::empty() {
		return data.empty() && tagName.empty();
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
	
	void XmlNode::setAttribute(XmlAttribute & attribute) {
		for (size_t i = 0; i < attributes.size(); i++) {
			XmlAttribute & attr = attributes[i];
			if (!attr.getName().compare(attribute.getName())) {
				attr.setValue(attribute.getValue());
				return;
			}
		}

		attributes.push_back(attribute);
	}
	
	XmlAttribute & XmlNode::getAttribute(const string & name) {
		for (size_t i = 0; i < attributes.size(); i++) {
			XmlAttribute & attr = attributes[i];
			if (!attr.getName().compare(name)) {
				return attr;
			}
		}
		return EMPTY_ATTRIBUTE;
	}

	string XmlNode::getAttributeValue(const string & name) {
		return getAttribute(name).getValue();
	}

	vector<XmlAttribute> & XmlNode::getAttributes() {
		return attributes;
	}

	string XmlNode::getFirstContent() {
		return children.size() > 0 ? children[0].getData() : "";
	}

	bool XmlNode::isTextNode() {
		return !data.empty();
	}
	
	bool XmlNode::isElementNode() {
		return !tagName.empty();
	}

	int XmlNode::getChildrenElementNodeCount() {
		int cnt = 0;
		LOOP_VEC(children, i) {
			XmlNode & node = children[i];
			if (node.isElementNode()) {
				cnt++;
			}
		}
		return cnt;
	}
	
	XmlNode& XmlNode::operator[](size_t index) {
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
    
    XmlNode & XmlDocument::getRootNode() {
        return rootNode;
    }

	void XmlDocument::setRootNode(XmlNode & rootNode) {
		this->rootNode = rootNode;
	}

    void XmlDocument::setPrologue(const string & prologue) {
        this->prologue = prologue;
    }
}
