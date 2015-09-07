// -*- C++ -*-
/*
  Defines the abstract idea of an atomic predicate.

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

#ifndef __AtomicPredicate_h_
#define __AtomicPredicate_h_

#include "typedef.h"
#include <vector>

#include "bit_vector.h"

using namespace std;

class AtomicPredicate {
public:
  typedef vector<relativePosType> position_vector;
  typedef featureIndexType1D storage_vector;

  AtomicPredicate() {
  }
  virtual ~AtomicPredicate() {}

  // The test:
  virtual bool test(const wordType2D&, int, const wordType) const = 0;
  virtual double test(const wordType2D&, int, const wordType, const float2D& context_prob) const = 0;
  // The instanciation of features
  virtual void instantiate(const wordType2D& corpus, int sample_ind, wordTypeVector& instances) const = 0;
  virtual void identify_strings(const wordType1D& word_id, wordType_set& words) const = 0;
  virtual void identify_strings(wordType word_id, wordType_set& words) const = 0;

  virtual string printMe(wordType wrd) const = 0;

  virtual void set_dependencies(vector<bit_vector>&) const = 0;

  virtual void get_sample_differences(position_vector& positions) const = 0;
  virtual void get_feature_ids(storage_vector& features) const = 0;
  virtual bool is_indexable() const = 0;
};

inline bool AtomicPredicate::test(const wordType2D&, int, const wordType) const {
  return false;
}
inline double AtomicPredicate::test(const wordType2D&, int, const wordType, const float2D&) const {
  return 0;
}

inline void AtomicPredicate::identify_strings(const wordType1D& word_id, wordType_set& words) const {};

inline void AtomicPredicate::identify_strings(wordType word_id, wordType_set& words) const {};

inline void AtomicPredicate::instantiate(const wordType2D& corpus, int sample_ind, wordTypeVector& instances) const {
}

inline string AtomicPredicate::printMe(wordType wrd) const {
  return "";
}

inline void AtomicPredicate::set_dependencies(vector<bit_vector>&) const{
}

inline bool AtomicPredicate::is_indexable() const {
  return false;
}
#endif
