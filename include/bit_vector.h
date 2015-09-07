#ifndef __BIT_VECTOR_H__
#define __BIT_VECTOR_H__

#include "gcc_version.h"

#if HAVE_GCC_VERSION(3,4)
typedef std::vector<bool> bit_vector; 
#else
typedef std::bit_vector bit_vector;
#endif

#endif
