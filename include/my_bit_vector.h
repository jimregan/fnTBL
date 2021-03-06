// -*- C++ -*-
/*
  Defines an extension of bit_vector that can compute easily the
  number of bits that are set underbelow a given index.

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

#ifndef __my_bit_vector_h
#define __my_bit_vector_h

#include <vector>
#include <cassert>
#include <iostream>
#include "gcc_version.h"
#include "debug.h"

template <class int_type = unsigned short>
class my_bit_vector: public std::vector<bool> {
  typedef std::vector<bool> _Base ;
//   mutable int_type last_pos, last_val;

public:
  explicit my_bit_vector()
    : _Base() 
  {
    reset();
  }

  explicit my_bit_vector(size_type __n, bool __value)
    : _Base(__n, __value)
  {
    reset();
  }

  explicit my_bit_vector(size_type __n)
    : _Base(__n)
  {
    reset();
  }

  explicit my_bit_vector(const my_bit_vector& __x) : _Base(__x){
    reset();
  }

  void reset() {
    // 	last_pos = static_cast<int_type>(-1);
    // 	last_val = 0;
  }

  int count_bits_before_pos(unsigned int pos) const {
    int_type b = 0, cnt = 0, shft = 0;
    static char no_bits[256] = {
      0,1,1,2,1,2,2,3,
      1,2,2,3,2,3,3,4,
      1,2,2,3,2,3,3,4,
      2,3,3,4,3,4,4,5,
      1,2,2,3,2,3,3,4,
      2,3,3,4,3,4,4,5,
      2,3,3,4,3,4,4,5,
      3,4,4,5,4,5,5,6,
      1,2,2,3,2,3,3,4,
      2,3,3,4,3,4,4,5,
      2,3,3,4,3,4,4,5,
      3,4,4,5,4,5,5,6,
      2,3,3,4,3,4,4,5,
      3,4,4,5,4,5,5,6,
      3,4,4,5,4,5,5,6,
      4,5,5,6,5,6,6,7,
      1,2,2,3,2,3,3,4,
      2,3,3,4,3,4,4,5,
      2,3,3,4,3,4,4,5,
      3,4,4,5,4,5,5,6,
      2,3,3,4,3,4,4,5,
      3,4,4,5,4,5,5,6,
      3,4,4,5,4,5,5,6,
      4,5,5,6,5,6,6,7,
      2,3,3,4,3,4,4,5,
      3,4,4,5,4,5,5,6,
      3,4,4,5,4,5,5,6,
      4,5,5,6,5,6,6,7,
      3,4,4,5,4,5,5,6,
      4,5,5,6,5,6,6,7,
      4,5,5,6,5,6,6,7,
      5,6,6,7,6,7,7,8
    };

    if(pos<=0)
      return 0;

#ifdef USE_TYPE1 // struct __bit_iterator
#if __GNUC__ < 3
  unsigned *const my_bit_array = _M_start._M_p;
#else
  #if HAVE_GCC_VERSION(3,4)
    const unsigned long * my_bit_array = this->_M_impl._M_start._M_p;
  #else
    #if HAVE_GCC_VERSION(3,1)
	    const unsigned long * my_bit_array = _M_start._M_p;
    #else
	    const unsigned int* my_bit_array = _M_start._M_p;
    #endif
  #endif
#endif
#endif

#ifdef USE_TYPE2
    unsigned *const my_bit_array = start.p;
#endif

    if(pos<8)
      return no_bits[static_cast<unsigned char>(my_bit_array[0]) & ( (1U << (pos & 0x7)) - 1 )];

    int_type new_pos = pos & 0xfffffff8;
    int_type last_pos=static_cast<int_type>(-1), last_val=0;
    if(0 < static_cast<int>(last_pos) - new_pos && last_pos - new_pos < new_pos - 2) {
      b = last_pos-8;
      shft = b & 0x1f;
      cnt = last_val;
      while (b >= new_pos) {
	cnt -= no_bits[static_cast<unsigned char>(my_bit_array[b >> 5] >> shft)];
	b -= 8;
	shft = (shft+24) & 0x1f;
      }
      shft = (b+=8) & 0x1f;
    } else {
      if(new_pos >= last_pos && last_pos != static_cast<int_type>(-1)) {
	cnt = last_val;
	b = last_pos;
	shft = b & 0x1f;
      }
      while (b+8<=pos) {
	cnt += no_bits[static_cast<unsigned char>(my_bit_array[b >> 5] >> shft)];
	b += 8;
	shft = (shft+8) & 0x1f;
      }
    }		
    last_pos = pos & 0xfffffff8; 
    last_val = cnt;
    cnt += no_bits[static_cast<unsigned char>( (my_bit_array[b >> 5] >> shft) & 
					       ( (1U << (pos & 0x7)) - 1 )
					       ) ];
    ON_DEBUG(assert(last_val<=last_pos));
    return cnt;
    return 1;
  }

  bool read_in_binary_format(std::istream& istr, int size = -1);
  void write_in_binary_format(std::ostream& ostr, int size = -1) const;

  bool read_in_text_format(std::istream& istr, int size = -1);
  void write_in_text_format(std::ostream& ostr, int size = -1) const;

  static std::vector<char> bits;
  static char mask[8];
};

template <class int_type>
std::vector<char> my_bit_vector<int_type>::bits(0);

template <class int_type>
char my_bit_vector<int_type>::mask[8] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};

template <class int_type>
bool my_bit_vector<int_type>::read_in_binary_format(std::istream& istr, int size) {
  if(size == -1) { //if the size was not specified, is to be read from the stream
    if(! istr.read(reinterpret_cast<char*>(&size), sizeof(int)))
      return false;
  }

  resize(size);

#ifdef USE_TYPE1
#if HAVE_GCC_VERSION(3,4)
  std::_Bit_type * my_bit_array = this->_M_impl._M_start._M_p;
#else
  #if HAVE_GCC_VERSION(3,0)
    std::_Bit_type * my_bit_array = _M_start._M_p;
  #else
    unsigned int* my_bit_array = _M_start._M_p;
  #endif
#endif
#endif

#ifdef USE_TYPE2
  unsigned int * my_bit_array = start.p;
#endif

  unsigned int no_ints = (size >> 5) + ((size & 0x1f) > 0);
#if HAVE_GCC_VERSION(3,0)
  if(! istr.read(reinterpret_cast<char*>(my_bit_array), no_ints*sizeof(std::_Bit_type)))
#else
  if(! istr.read(reinterpret_cast<char*>(my_bit_array), no_ints*sizeof(unsigned int)))
#endif
    return false;
  reset();
  return true;
}

template <class int_type>
void my_bit_vector<int_type>::write_in_binary_format(std::ostream& ostr, int size) const {
  if(size != -1) // if size if specified, write it to the output stream
    ostr.write(reinterpret_cast<char*>(&size), sizeof(int));
  else
    size = this->size();
  
#ifdef USE_TYPE1
#if HAVE_GCC_VERSION(3,4)
  std::_Bit_type * my_bit_array = this->_M_impl._M_start._M_p;
#else
  #if HAVE_GCC_VERSION(3,0)
    std::_Bit_type * my_bit_array = _M_start._M_p;
  #else
    unsigned int* my_bit_array = _M_start._M_p;
  #endif
#endif
#endif

#ifdef USE_TYPE2
  unsigned int * my_bit_array = start.p;
#endif

  unsigned int no_ints = (size >> 5 ) + ((size & 0x1f) > 0);

#if HAVE_GCC_VERSION(3,0)
  ostr.write(reinterpret_cast<char*>(my_bit_array), no_ints*sizeof(std::_Bit_type));
#else
  ostr.write(reinterpret_cast<char*>(my_bit_array), no_ints*sizeof(unsigned int));
#endif
}
  
template <class int_type>
bool my_bit_vector<int_type>::read_in_text_format(std::istream& istr, int size) {
  if(size == -1) 
    if(! (istr >> size))
      return false;
  
  unsigned int no_bits = (size >> 3 ) + ((size & 0x07) > 0);
  bits.resize(no_bits);

  resize(size);
  char bt;
  for(iterator bit = begin() ; bit != end() ; ++bit) {
    if(! (istr >> bt) )
      return false;
    *bit = (bt=='0' ? false : true);
  }
  reset();
  return true;
}

template <class int_type>
void my_bit_vector<int_type>::write_in_text_format(std::ostream& ostr, int size) const {
  if(size != -1)
    ostr << size << " ";

  for(const_iterator bit = begin() ; bit != end() ; ++bit)
    ostr << ((*bit) ? '1' : '0') << std::flush;
}

#endif /* __my_bit_vector */
