#ifndef __UUID_HPP__
#define __UUID_HPP__

#include <string>

namespace UPNP {
    
    class Uuid {
    private:
        std::string uuid;
        std::string rest;
    public:
        Uuid(const std::string & uuid);
        virtual ~Uuid();
        
        void clear();
        void parse(const std::string & uuid);
        
        std::string getUuid() const;
        std::string getRest() const;
        
        std::string toString() const;
    };
}

#endif
