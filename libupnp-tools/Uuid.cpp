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

		size_t offset = 5;
        clear();
        
        if (!Text::startsWith(uuid, "uuid:")) {
			offset = 0;
        }
        
        size_t sep = uuid.find("::");
        if (sep != string::npos) {
            this->uuid = uuid.substr(offset, sep - offset);
            this->rest = uuid.substr(sep + 2);
        } else {
            this->uuid = uuid.substr(offset);
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
    
    void Uuid::testValidFormat(const std::string & uuid) {
        
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
    
    string Uuid::toString() const {
        return "uuid:" + uuid + (rest.empty() ? "" : "::" + rest);
    }

	
	UuidGenerator::UuidGenerator() {
	}
	UuidGenerator::~UuidGenerator() {
	}
	

	UuidGeneratorDefault::UuidGeneratorDefault() : nonce(0) {
	}
	UuidGeneratorDefault::~UuidGeneratorDefault() {
	}
	string UuidGeneratorDefault::generate() {
		unsigned long num = nonce++;
        
        string part1(8, 'a');
        string part2 = Text::toHexString(num);
        if (part2.length() < 4) {
            part2 = string(4 - part2.length(), '0') + part2;
        } else if (part2.length() > 4) {
            part2 = part2.substr(part2.length() - 4);
        }
        string part3(4, 'c');
        string part4(4, 'd');
        string part5 = Text::toHexString(tick_milli());
        if (part5.length() < 12) {
            part5 = string(12 - part5.length(), '0') + part5;
        }
        
        string uuid = part1 + "-" + part2 + "-" + part3 + "-" + part4 + "-" + part5;
        return uuid;
	}
}
