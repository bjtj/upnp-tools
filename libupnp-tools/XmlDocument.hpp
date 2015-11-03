#ifndef __XML_HPP__
#define __XML_HPP__

#include <string>
#include <vector>

namespace XML {
    
    /**
     * @brief xml namespace
     */
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

    /**
     * @brief xml attribute
     */
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
        std::string getNamespace() const;
		std::string getName() const;
		std::string getValue() const;
	};

    /**
     * @brief xml node
     */
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
        const std::string & getData() const;
        void setData(const std::string & data);
		const std::string & getNamespace() const;
		void setNamespace(const std::string & ns);
		const std::string & getTagName() const;
		void setTagName(const std::string & tagName);
        
        void setParent(XmlNode * parent);
        XmlNode * getParent();
        
        XmlNode * addNode(XmlNode & node);
        const XmlNode & getNode(size_t index) const;
		const std::vector<XmlNode> & getChildren() const;
		bool empty();
		size_t size();
        
		void setAttribute(const std::string & name, const std::string & value);
		void setAttribute(const XmlAttribute & attribute);
		std::string getAttributeValue(const std::string & name) const;
		const XmlAttribute & getAttribute(const std::string & name) const;
		const std::vector<XmlAttribute> & getAttributes() const;
        
		bool isTextNode() const;
		bool isElementNode() const;
		std::string getFirstContent() const;
		int getChildrenElementNodeCount() const;
        XmlNode getFirstElement() const;
		const XmlNode & operator[](size_t index) const;
	};

    /**
     * @brief xml document
     */
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
        const XmlNode & getRootNode() const;
		void setRootNode(XmlNode & rootNode);
        void setPrologue(const std::string & prologue);
	};

}

#endif
