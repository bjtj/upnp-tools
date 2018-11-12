#ifndef __UPNP_USN_HPP__
#define __UPNP_USN_HPP__

#include <string>

namespace upnp {
	
    /**
     * USN (Unique Service Name)
     */
    class USN {
    private:
	std::string _str;
    public:
	USN();
	USN(const std::string & str);
	virtual ~USN();
	std::string & str();
	std::string str() const;
	std::string uuid() const;
	std::string type() const;
	bool empty() const;
	std::string toString() const;
    };
}

#endif
