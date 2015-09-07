// -*- c++ -*-
/*
  Defines a class needed for the featureExtractor program

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

#ifndef _sample_signature_h_
#define _sample_signature_h_

#if __GNUG__ < 3
#include <hash_set>
#else
#include <ext/hash_set>
#endif /* __GNUG__ */

class sample_signature {
  slist<int> preds;
  int hash_value;
public:
  sample_signature():hash_value(0) {}
  sample_signature(const sample_signature& x): preds(x.preds), hash_value(x.hash_value) {}
  sample_signature(const set<int>& v):hash_value(0) {
    for(set<int>::const_iterator i=v.begin() ; i!=v.end() ; i++) {
      hash_value = 5*hash_value + *i;
      preds.insert(preds.end(),*i);
    }
  }

  int hash_val() const {
    return hash_value;
  }

  void push_back(int x) {
    preds.insert(preds.end(), x);
    hash_value = 5*hash_value+x;
  }

  bool operator== (const sample_signature& x) {
    return hash_value == x.hash_value && preds == x.preds;
  }
};

namespace std {
  struct hash<sample_signature> {
    size_t operator() (const sample_signature& s) const {
      return s.hash_val();
    }
  };
}

#endif
