#include "Uuid.hpp"
#include <liboslayer/Text.hpp>

namespace UPNP {
    
    using namespace std;
    using namespace UTIL;
    
    Uuid::Uuid(const string & uuid) {
        parse(uuid);
        
    }
    Uuid::~Uuid() {
        
    }
    
    void Uuid::clear() {
        uuid.clear();
        rest.clear();
    }
    
    void Uuid::parse(const string & uuid) {
        
        clear();
        
        if (!Text::startsWith(uuid, "uuid:")) {
            throw -1;
        }
        
        size_t sep = uuid.find("::");
        if (sep != string::npos) {
            this->uuid = uuid.substr(0, sep);
            this->rest = uuid.substr(sep + 2);
        } else {
            this->uuid = uuid;
        }
    }
    
    string Uuid::getUuid() const {
        return uuid;
    }
    
    string Uuid::getRest() const {
        return rest;
    }
    
    void Uuid::setUuid(const std::string & uuid) {
        this->uuid = uuid;
    }
    
    void Uuid::setRest(const std::string & rest) {
        this->rest = rest;
    }
    
    string Uuid::toString() const {
        return uuid + (rest.empty() ? "" : "::" + rest);
    }
}
