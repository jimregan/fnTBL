// -*- C++ -*- 
/*
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
  
#ifndef __my_pair_h_
#define __my_pair_h_

#if __GNUG__ < 3
#include <pair.h>
#else
#include <bits/stl_pair.h>
#endif

template<class T1, class T2>
struct m_pair {
  typedef T1 first_type;
  typedef T2 second_type;

  T1 first;
  T2 second;
  m_pair() : first(T1()), second(T2()) {}
  m_pair(const T1 a, const T2 b) : first(a), second(b) {}

#ifdef __STL_MEMBER_TEMPLATES
  template <class U1, class U2>
  m_pair(const m_pair<U1, U2>& p) : first(p.first), second(p.second) {}

//   pair<T1, T2> operator pair<T1, T2> () {
// 	return make_pair(first, second);
//   }
#endif
};

template <class T1, class T2>
inline bool operator==(const m_pair<T1, T2>& x, const m_pair<T1, T2>& y) { 
  return x.first == y.first && x.second == y.second; 
}

template <class T1, class T2>
inline bool operator<(const m_pair<T1, T2>& x, const m_pair<T1, T2>& y) { 
  return x.first < y.first || (!(y.first < x.first) && x.second < y.second); 
}

template <class T1, class T2>
inline m_pair<T1, T2> make_m_pair(const T1 x, const T2 y) {
  return m_pair<T1, T2>(x, y);
}

#endif

