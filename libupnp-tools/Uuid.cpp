#include "Uuid.hpp"
#include <liboslayer/os.hpp>
#include <liboslayer/Text.hpp>
#include <vector>

namespace UPNP {
    
    using namespace std;
    using namespace OS;
    using namespace UTIL;
    
    DECL_NAMED_ONLY_EXCEPTION(WrongFormatException);
    
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
    
    void Uuid::validFormat(const std::string & uuid) {
        
        if (uuid.empty()) {
            throw WrongFormatException("wrong format/empty string", -1, 0);
        }
        
        vector<string> parts = Text::split(uuid, "-");
        if (parts.size() != 5) {
            throw WrongFormatException("wrong format", -1, 0);
        }
        
        if (parts[0].length() != 8) {
            throw WrongFormatException("wrong format", -1, 0);
        }
        if (parts[1].length() != 4) {
            throw WrongFormatException("wrong format", -1, 0);
        }
        if (parts[2].length() != 4) {
            throw WrongFormatException("wrong format", -1, 0);
        }
        if (parts[3].length() != 4) {
            throw WrongFormatException("wrong format", -1, 0);
        }
        if (parts[4].length() != 12) {
            throw WrongFormatException("wrong format", -1, 0);
        }
        
        for (size_t i = 0; i < parts.size(); i++) {
            if (parts[i].find_first_not_of("0123456789abcdefABCDEF") != string::npos) {
                throw WrongFormatException("wrong format/hex digit", -1, 0);
            }
        }
    }
    
    string Uuid::generateUuid() {
        return "";
    }
    
    string Uuid::toString() const {
        return uuid + (rest.empty() ? "" : "::" + rest);
    }
}
