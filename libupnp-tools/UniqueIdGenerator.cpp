#include "UniqueIdGenerator.hpp"

namespace UPNP {
    
    UniqueIdGenerator::UniqueIdGenerator() : id(0), sem(1) {
    }
    
    UniqueIdGenerator::UniqueIdGenerator(ID_TYPE initial) : id(initial), sem(1) {
    }
    
    UniqueIdGenerator::~UniqueIdGenerator() {
    }
    
    ID_TYPE UniqueIdGenerator::generate() {
        OS::AutoLock lock(sem);
        return id++;
    }
}