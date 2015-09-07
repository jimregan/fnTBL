#ifndef __hash_wrapper_h__
#define __hash_wrapper_h__

#if __GNUG__ < 3
#include <hashtable.h>
#include <hash_map>
#include <hash_set>
#define HASH_NAMESPACE std
#else
#include <ext/hash_map>
#include <ext/hash_set>
#define HASH_NAMESPACE __gnu_cxx
#endif 

#endif
