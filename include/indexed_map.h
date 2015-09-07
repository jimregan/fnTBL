// -*- C++ -*-
/*
  Defines a container of elements pretty much like a vector, but
  the elements can be accesses as a map as well (bidirectional map).
  There is a requirement on the types: the indexed type cannot be an
  integral type.

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

#ifndef __indexed_map_h__
#define __indexed_map_h__

#include <vector>
#include "hash_wrapper.h"
#include <map>


template <class index2, class index1 = unsigned, class map_type = HASH_NAMESPACE::hash_map<index2, index1> > 
class indexed_map {
public:
  typedef indexed_map<index2, index1, map_type> self;
  typedef std::vector<index2*> rep_type;

//   typedef rep_type::iterator iterator;
//   typedef rep_type::const_iterator const_iterator;
  typedef int size_type;

  template <class Value, class Ref, class Ptr, class underlying_iterator>
  class generic_iterator {
  public:
	typedef Value value_type;
	typedef int difference_type;
	typedef Ref reference;
	typedef Ptr pointer;
	typedef generic_iterator<Value, Ref, Ptr, underlying_iterator> self;

	generic_iterator(underlying_iterator i): it(i)  {}

	generic_iterator(const self& i): it(i.it) {}
	
	~generic_iterator() {}

	Ref operator*() {
	  return **it;
	}

	Ptr operator ->() {
	  return *it;
	}

	self& operator++() {
	  ++it;
	  return *this;
	}

	self operator++ (int) {
	  self t = *this;
	  ++it;
	  return t;
	}

	self& operator-- () {
	  --it;
	  return *it;
	}

	self operator-- (int) {
	  self temp = *this;
	  --it;
	  return temp;
	}

	bool operator== (const self& other) const {
	  return it == other.it;
	}

	bool operator != (const self& other) const {
	  return it != other.it;
	}

	friend difference_type operator- (const self& it1, const self& it2) {
	  return it1.it - it2.it;
	}

  private:
	underlying_iterator it;
  };

  typedef generic_iterator<index2, index2&, index2*, typename std::vector<index2*>::iterator > iterator;
  typedef generic_iterator<const index2, const index2&, const index2*, typename std::vector<index2*>::const_iterator > const_iterator;

  indexed_map(): fake_index(-1) {}

  // This should not be called at all...
private:
  indexed_map(const self& ra):index_map(ra.index_map), storage(ra.storage), available_inds(ra.available_inds) {}
  
public:
  index1 insert(const index2& elem) {
	typename map_type::iterator f = index_map.find(elem);
	if(f == index_map.end()) {
	  if(available_inds.size()>0) {
		f = index_map.insert(make_pair(elem, available_inds.back())).first;
		storage[available_inds.back()] = & const_cast<index2&>(f->first);
 		available_inds.pop_back();
	  } else {
		f = index_map.insert(make_pair(elem, storage.size())).first;
		storage.push_back(& const_cast<index2&>(f->first));
	  }
	}
	return f->second;
  }

  size_type size() const {
	return storage.size() - available_inds.size();
  }

  index1 reverse_access(const index2& elem) const {
	typename map_type::const_iterator f = index_map.find(elem);
	if(f == index_map.end()) {
	  return -1;
	} else
	  return f->second;
  }	

  index1 operator [] (const index2& elem) const {
	return reverse_access(elem);
  }

  const index2& direct_access(index1 ind) const {
	if (ind == fake_index || ind>=storage.size())
	  return fake_elem;
	
	return *storage[ind];
  }

  const index2& operator [] (index1 ind) const {
	return direct_access(ind);
  }

  const index2& direct_access(index1 ind) {
	if (ind == fake_index || ind>=storage.size())
	  return fake_elem;
	
	return *storage[ind];
  }	

  const index2& operator [] (index1 ind) {
	return direct_access(ind);
  }

  void remove_element(const index2& elem) {
	typename map_type::iterator f = index_map.find(elem);
	if(f != index_map.end()) {
	  available_inds.push_back(f->second);
	  index_map.erase(f);
	  storage[f->second] = 0;
	}
  }

  iterator begin() {
	return storage.begin();
  }

  iterator end() {
	return storage.end();
  }

  const_iterator begin() const {
	return storage.begin();
  }

  const_iterator end() const {
	return storage.end();
  }

  iterator find(const index2& elem) {
	typename map_type::iterator it = index_map.find(elem);
	if(it == index_map.end())
	  return end();
	else
	  return storage.begin() + it->second;
  }

  const_iterator find(const index2& elem) const {
	typename map_type::const_iterator it = index_map.find(elem);
	if(it == index_map.end())
	  return end();
	else
	  return storage.begin() + it->second;
  }

//   template <class iterator_type>
//   void erase(iterator_type i) {
// 	remove_element(*i);
//   }

  void erase(index1 ind) {
	remove_element(*storage[ind]);
  }

  void clear() {
	storage.clear();
	index_map.clear();
  }

  void destroy() {
	std::vector<index2*> tmp1;
	storage.swap(tmp1);
	std::vector<index1> tmp2;
	available_inds.swap(tmp2);
	map_type tmp3;
	index_map.swap(tmp3);
  }

  index2 fake_elem;
  index1 fake_index;

protected:
  map_type index_map;
  std::vector<index2*> storage;
  std::vector<index1> available_inds;
};

#endif
