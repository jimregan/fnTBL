// -*- c++ -*-
/*
  svector is a container to represent vectors that are not resizable 
  (uses less storage).

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

#ifndef __svector_h__
#define __svector_h__

#include <vector>
#include "sized_memory_pool.h"
#include "debug.h"
#include "common.h"

template <class type, class size_type = unsigned short> 
class svector {
public:
  typedef svector<type> self;
  typedef type* iterator;
  typedef const type* const_iterator;
  typedef type data_type;
  typedef type& ref_type;
  typedef type* pointer_type;
  typedef const type* const_pointer_type;

  svector(): _size(0), data(0)  {
    ON_DEBUG(assert(valid()));
  }

  svector(int n): _size(n) {
    allocate_data();
    ON_DEBUG(assert(valid()));	
  }

  svector(const self& sv): _size(sv._size) {
    allocate_data();
    std::copy(sv.data, sv.data+_size, data);
    ON_DEBUG(assert(valid()));
  }

  svector(const std::vector<data_type>& v): _size(v.size()) {
    allocate_data();
    std::copy(v.begin(), v.end(), data);
    ON_DEBUG(assert(valid()));
  }

  ~svector() {
    free_data();
    data = 0;
    _size = 0;
  }

  ref_type operator [] (int i) {
    ON_DEBUG(assert(valid()));
    return data[i];
  }

  const ref_type operator [] (int i) const {
    return data[i];
  }

  iterator begin() {
    ON_DEBUG(assert(valid()));
    return data;
  }

  const_iterator begin() const {
    ON_DEBUG(assert(valid()));
    return data;
  }

  iterator end() {
    ON_DEBUG(assert(valid()));
    return data+_size;
  }

  const_iterator end() const {
    ON_DEBUG(assert(valid()));
    return data+_size;
  }

  self& operator = (const self& sv) {
    if(this != &sv) {
      resize(sv._size);
      std::copy(sv.data, sv.data+_size, data);
    }
    ON_DEBUG(assert(valid()));
    return *this;
  }

  bool valid() const {
    return ! (_size==0 ^ data==0);
  }

  void resize(int sz) {
    free_data();
    _size = sz;
    allocate_data();
  }

  void clear() {
    free_data();
    data = 0;
    _size = 0;
  }

  unsigned int size() const {
    return _size;
  }

protected:
  static sized_memory_pool<type> memory_pool;
  static const int _MAX_SIZE = 100;

  void allocate_data() {
    data = memory_pool.allocate(_size);
    ON_DEBUG(assert(valid()));
  }

  void free_data() {
    if(data)
      memory_pool.deallocate(data, _size);
  }

public:

  static void deallocate_all() {
    memory_pool.destroy();
  }

  static void free_it() {
    memory_pool.dump();
  }

protected:
  size_type _size;
  type* data;
};

template <class type, class size_type>
inline bool 
operator < (const svector<type, size_type>& v1, const svector<type, size_type>& v2) {
  return lexicographical_compare(v1.begin(), v1.end(),
				 v2.begin(), v2.end());
}

template <class type, class size_type>
sized_memory_pool<type> svector<type, size_type>::memory_pool(_MAX_SIZE);

template<class type, class size_type>
std::ostream& operator <<(std::ostream& ostr, const svector<type, size_type>& sv) {
  ostr << sv.size();
  for(int i=0 ; i<sv.size() ; i++)
    ostr << " " << sv[i];
  return ostr;
}

template <class type, class size_type>
std::istream& operator >> (std::istream& istr, svector<type, size_type>& sv) {
  int sz;
  istr >> sz;
  sv.resize(sz);
  for(int i=0 ; i<sz ; i++)
    istr >> sv[i];
  return istr;
}

#endif
