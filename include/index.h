// -*- C++ -*-
/*
  Defines an index - an inversed index file for the tokens that appear in the data.

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

#ifndef _index_h__
#define _index_h__

#include <vector>

#if __GNUG__ < 3
#include "memory.h"
#include <slist>
#else
#include <ext/slist>
#include <memory>
#endif /* __GNUG__ */

#include <utility>
#include "typedef.h"
#include "common.h"
#include "indexed_map.h"
#include "hash_wrapper.h"

#include <cassert>

template <typename T1, typename T2>
class word_index;

template <typename T1=unsigned int, typename T2=unsigned short>
class word_index_reference {
public:
  typedef unsigned int rep_type;
  typedef word_index_reference<rep_type> self;  
  word_index_reference(word_index<T1, T2>& p, rep_type i):parent_index(p), index(i) {
  }

  T1 line_id() {
    return parent_index.pair_map[index].first;
  }

  T2 word_id() {
    return parent_index.pair_map[index].second;
  }

private:
  word_index<T1, T2>& parent_index;
  rep_type index;
};

template <typename T1=unsigned int, typename T2=unsigned short>
class word_index_iterator {
public:
  //   typedef std::pair<T1, T2> rep_type;
  typedef word_index_reference<T1, T2> rep_type;
  typedef word_index_iterator<T1, T2> self;
  typedef std::vector<T1> slist_type;
  typedef std::set<T1> set_type;
  typedef word_index<T1, T2> index_type;

  word_index_iterator(index_type& p, typename slist_type::iterator it): parent(p), type(0) {
    real_it.slist_it = it;
  }

  word_index_iterator(index_type& p, typename set_type::iterator it): parent(p), type(1) {
    real_it.set_it = it; 
  }

  rep_type operator*() {
    switch(type) {
    case 0:
    case 2:
      return rep_type(parent, *real_it.slist_it);
    case 1:
      return rep_type(parent, *real_it.set_it);
    default:
      return rep_type(parent, *real_it.slist_it);
    }	  
  }

  std::auto_ptr<rep_type> operator ->() const {
    switch(type) {
    case 1:
      return std::auto_ptr<rep_type>(new rep_type (parent, *real_it.set_it));
    case 0:
    case 2:
    default:
      return std::auto_ptr<rep_type>(new rep_type (parent, *real_it.slist_it));
    }
  }

  self& operator++() {
    switch(type) {
    case 0:
    case 2:
      real_it.slist_it++;
      break;
    case 1:
      real_it.set_it++;
      break;
    }
    return *this;
  }

  self operator++(int) {
    self temp = *this;
    switch(type) {
    case 0:
    case 2:
      real_it.slist_it++;
      break;
    case1:
      real_it.set_it++;
      break;
    }
    return temp;
  }

  bool operator == (const self& ot) const {
    if(type != ot.type)
      return false;

    switch(type) {
    case 0:
    case 2:
      return real_it.slist_it == ot.real_it.slist_it;
    case 1:
      return real_it.set_it == ot.real_it.set_it;
    default:
      return real_it.slist_it == ot.real_it.slist_it;
    }
  }

  bool operator != (const self& ot) const {
    return !operator == (ot);
  }

protected:
  word_index<T1, T2>& parent;
  struct {
    typename slist_type::iterator slist_it;
    typename set_type::iterator set_it;
  } real_it;
  int type;
};

template <typename T1=unsigned int, typename T2=unsigned short>
class word_index {
  friend class word_index_iterator<T1, T2>;
  friend class word_index_reference<T1, T2>;
public:
  typedef T1 index_type1;
  typedef T2 index_type2;
  typedef std::pair<index_type1, index_type2> indices_pair_type;
  typedef indexed_map<indices_pair_type, unsigned int> bimap_type;

  typedef std::vector<unsigned int> slist_type;
  typedef std::set<unsigned int> set_type;

  typedef word_index_iterator<T1, T2> iterator;
  typedef word_index<T1, T2>   self;

  word_index(int t=0): type(t) {
    assert(t==0 || t==1 || t==2);
  }
  ~word_index() {}

  iterator begin(int n) {
    switch(type) {
    case 2:
      n = 0;
    case 0:
      if(n>=data.slist_field.size())
	return iterator(*this, static_cast<slist_type::iterator>(fake.slist_field.end()));
      else 
	return iterator(*this, data.slist_field[n].begin());
    case 1:
      if(n>=data.set_field.size())
	return iterator(*this, fake.set_field.end());
      else
	return iterator(*this, data.set_field[n].begin());
    default:
      if(n>=data.slist_field.size())
	return iterator(*this, fake.slist_field.end());
      else 
	return iterator(*this, data.slist_field[n].end());	  
    }
  }

  iterator end(int n) {
    switch(type) {
    case 2:
      n = 0;
    case 0:
      if(n>=data.slist_field.size())
	return iterator(*this, fake.slist_field.end());
      else 
	return iterator(*this, data.slist_field[n].end());
    case 1:
      if(n>=data.set_field.size())
	return iterator(*this, fake.set_field.end());
      else 
	return iterator(*this, data.set_field[n].end());
    default:
      if(n>=data.slist_field.size())
	return iterator(*this, fake.slist_field.end());
      else 
	return iterator(*this, data.slist_field[n].end());	  
    }
  }

  iterator find(int n, int i, int j) {
    switch(type) {
    case 1:
      if(n>=data.set_field.size())
	return iterator(*this, fake.set_field.end());
      return iterator(*this, data.set_field[n].find(pair_map[std::make_pair(i,j)]));
    case 2:
      n = 0;
    case 0:
    default:
      return begin(n);
    }
  }

  void resize(int new_size) {
    switch (type) {
    case 2:
      new_size = 1;
    case 0:
      data.slist_field.resize(new_size);
      break;
    case 1:
      data.set_field.resize(new_size);
      break;
    }
  }

  void resize(int wrd_index, int new_size) {
    if(type == 2)
      wrd_index = 0;
    if(type==0 || type==2)
      data.slist_field[wrd_index].resize(new_size);
  }

  void clear(int wrd_index) {
    switch (type) {
    case 2:
      wrd_index = 0;
    case 0:
      data.slist_field[wrd_index].clear();
      break;
    case 1:
      data.set_field[wrd_index].clear();
      break;
    }
  }

  void clear() {
    switch (type) {
    case 2:
    case 0:
      data.slist_field.clear();
      break;
    case 1:
      data.set_field.clear();
      break;
    }
  }

  void insert(int wrd_index, const index_type1& t1, const index_type2& t2) {
    switch (type) {
    case 2:
      wrd_index = 0;
    case 0:
      if(data.slist_field.size() <= wrd_index) 
	data.slist_field.resize(wrd_index+1);
      data.slist_field[wrd_index].push_back(pair_map.insert(make_pair(t1,t2)));
      break;
    case 1:
      if(data.set_field.size() <= wrd_index)
	data.set_field.resize(wrd_index+1);
      data.set_field[wrd_index].insert(pair_map.insert(make_pair(t1, t2)));
      break;
    }
  }

  void erase(int wrd_index, const index_type1& t1, const index_type2& t2) {
    switch (type) {
    case 2:
      wrd_index = 0;
    case 0:
      std::cerr << "Error - we were not supposed to remove elements from slist kind of a index!" << "\n";
      break;
    case 1:
      if(wrd_index>data.set_field.size())
	std::cerr << "Error - trying to erase an element that does not exits" << "\n";
      else
	data.set_field[wrd_index].erase(pair_map[make_pair(t1, t2)]);
      break;
    }
  }

  void copy_data_field(const self& obj, int wrd_index) {
    pair_map = obj.pair_map;
    switch (type) {
    case 2:
      wrd_index = 0;
    case 0:
      if(data.slist_field.size()<=wrd_index)
	data.slist_field.resize(wrd_index+1);
      data.slist_field[wrd_index] = obj.data.slist_field[wrd_index];
      break;
    case 1:
      if(data.set_field.size() <= wrd_index) 
	data.set_field.resize(wrd_index+1);
      data.set_field[wrd_index] = obj.data.set_field[wrd_index];
      break;
    }
  }

  int get_type() const {
    return type;
  }

  static void create_map(std::vector<int>& counts) {
    pair_map.clear();
    for(unsigned int i=0 ; i<counts.size() ; ++i)
      for(unsigned short int j=0 ; j<counts[i] ; ++j)
	pair_map.insert(std::make_pair(i, j));
  }

  void finalize() {
    switch(type) {
    case 0:
      for(int i=0 ; i<data.slist_field.size() ; i++) {
	slist_type temp = data.slist_field[i];
	temp.swap(data.slist_field[i]);
      }
      break;
    case 2:
      data.slist_field.resize(1);
      data.slist_field[0].resize(pair_map.size());
      iota(data.slist_field[0].begin(), data.slist_field[0].end(), 0);
      break;
    case 1:
      break;
    }
  }

  void compute_sizes(int& s1, int& s2) {
    s1 = s2 = 0;
    for(int i=0 ; i<data.slist_field.size() ; i++) {
      s1 += data.slist_field[i].size();
      s2 += data.slist_field[i].capacity();
    }
  }

protected:
  struct {
    std::vector<slist_type> slist_field;
    std::vector<set_type> set_field;
  } data;
  
  struct {
    slist_type slist_field;
    set_type set_field;
  } fake;
  
  unsigned short int type;
  static indexed_map<std::pair<T1, T2> > pair_map;
};

namespace HASH_NAMESPACE {
  template <typename T1, typename T2>
  struct hash<std::pair<T1, T2> > {
    size_t operator() (const std::pair<T1, T2>& p) const {
      return 17*p.first + p.second;
    }
  };
}

template <typename T1, typename T2>
indexed_map<std::pair<T1, T2> > word_index<T1, T2>::pair_map;
#endif
