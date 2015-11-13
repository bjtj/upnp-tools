#ifndef __UNIQUE_ID_GENERATOR_HPP__
#define __UNIQUE_ID_GENERATOR_HPP__

#include <liboslayer/os.hpp>

namespace UPNP {
    
    typedef unsigned long ID_TYPE;
    
    class UniqueIdGenerator {
    private:
        ID_TYPE id;
        OS::Semaphore sem;
    public:
        UniqueIdGenerator();
        UniqueIdGenerator(ID_TYPE initial);
        virtual ~UniqueIdGenerator();
        ID_TYPE generate();
    };
}

#endif
