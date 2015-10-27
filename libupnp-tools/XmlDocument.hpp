#ifndef __XML_HPP__
#define __XML_HPP__

#include <string>
#include <vector>

namespace XML {
    
    class XmlNamespace {
    private:
        std::string alias;
        std::string uri;
    public:
        XmlNamespace();
        virtual ~XmlNamespace();
        
        std::string getAlias();
        std::string getUri();
    };

	class XmlAttribute {
	private:
        std::string ns;
		std::string name;
		std::string value;
	public:
		XmlAttribute();
		XmlAttribute(std::string name, std::string value);
		virtual ~XmlAttribute();

        void setNamespace(const std::string & ns);
		void setName(const std::string & name);
		void setValue(const std::string & value);
        std::string getNamespace();
		std::string getName();
		std::string getValue();
	};

	class XmlNode {
	private:
        std::string data;
		std::string ns;
		std::string tagName;
		std::vector<XmlAttribute> attributes;
        std::vector<XmlNode> children;
        XmlNode * parent;
		
	public:
        XmlNode();
        virtual ~XmlNode();
        std::string & getData();
        void setData(const std::string & data);
		std::string & getNamespace();
		void setNamespace(const std::string & ns);
		std::string & getTagName();
		void setTagName(const std::string & tagName);
        void setParent(XmlNode * parent);
        XmlNode * getParent();
        XmlNode * addNode(XmlNode & node);
        XmlNode & getNode(size_t index);
		std::vector<XmlNode> & getChildren();
		bool empty();
		size_t size();
		void setAttribute(const std::string & name, const std::string & value);
		void setAttribute(XmlAttribute & attribute);
		std::string getAttributeValue(const std::string & name);
		XmlAttribute & getAttribute(const std::string & name);
		std::vector<XmlAttribute> & getAttributes();
		bool isTextNode();
		bool isElementNode();
		std::string getFirstContent();
		int getChildrenElementNodeCount();
		XmlNode & operator[](size_t index);
	};

	class XmlDocument {
	private:
        std::string prologue;
        std::vector<XmlNamespace> xmlNamespaces;
		XmlNode rootNode;
		
	public:
		XmlDocument();
		virtual ~XmlDocument();
        
        void addNamespacePhase(std::string & nsPhrase);
        XmlNamespace getNamespaceWithAlias(const std::string & alias);
        XmlNode & getRootNode();
		void setRootNode(XmlNode & rootNode);
        void setPrologue(const std::string & prologue);
	};

}

#endif
