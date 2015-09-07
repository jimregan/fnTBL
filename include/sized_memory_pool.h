// -*- C++ -*-
/*
  Defines a memory allocator - will store the deallocated memory
  for fast reallocation.

  This file is part of the fnTBL distribution.

  Copyright (c) 2001 Johns Hopkins University and Radu Florian and Grace Ngai.

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software, fnTBL version 1.0, and associated
  documentation files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use, copy,
  modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished
  to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  
*/

#ifndef _sized_memory_pool_h__
#define _sized_memory_pool_h__

#include <vector>

template <class type>
class sized_memory_pool {
  typedef std::vector<type*> ptype_vector;
  typedef std::vector<ptype_vector > rep_type;
public:
  typedef sized_memory_pool<type> self;
  sized_memory_pool(int sz): memory_pool(sz) {}

private:
  sized_memory_pool(const self& mp) {}

public:
  ~sized_memory_pool() {
    clear();
    memory_pool.resize(0);
  }

  type* allocate(int n) {
    if(n==0)
      return 0;

    if(n<memory_pool.size() && memory_pool[n].size()>0) {
      type* data = memory_pool[n].back();
      memory_pool[n].pop_back();
      return data;
    } 
    else 
      return new type [n];
  }

  void deallocate(type* data, int n) {
    if(n<memory_pool.size() && n>0)
      memory_pool[n].push_back(data);
    else {
      delete [] data;
      data = 0;
    }
  }

  void swap(self& mp) {
    memory_pool.swap(mp.memory_pool);
  }

  void destroy() {
    self temp(0);
    swap(temp);
  }

  void clear() {
    for(typename rep_type::iterator i=memory_pool.begin() ; i!=memory_pool.end() ; ++i)
      for(typename ptype_vector::iterator j=i->begin() ; j!=i->end() ; ++j)
	delete [] *j;
    memory_pool.clear();
  }

  void dump() {
    memory_pool.clear();
  }

protected:
  rep_type memory_pool;  
};

#endif
