#ifndef __MACROS_HPP__
#define __MACROS_HPP__

#define LOOP_VEC(VEC,INDEX) for (size_t INDEX = 0; INDEX < VEC.size(); INDEX++)
#define LOOP_MAP(MAP,T1,T2,ITER) for (std::map< T1, T2 >::iterator ITER = MAP.begin(); ITER != MAP.end(); ITER++)
#define LOOP_CONST_MAP(MAP,T1,T2,ITER) for (std::map< T1, T2 >::const_iterator ITER = MAP.begin(); ITER != MAP.end(); ITER++)
#define FOREACH_VEC(VEC,TYPE,ITER) for (std::vector< TYPE >::iterator ITER = VEC.begin(); ITER != VEC.end(); ITER++)
#define FOREACH_CONST_VEC(VEC,TYPE,ITER) for (std::vector< TYPE >::const_iterator ITER = VEC.begin(); ITER != VEC.end(); ITER++)

namespace UPNP {
}

#endif