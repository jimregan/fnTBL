// -*- C++ -*-
/*
  Defines a type of atomic predicate - returns true if the word argument
  contains a given character.

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

#ifndef __SubwordPartPredicate_h__
#define __SubwordPartPredicate_h__

#include "SingleFeaturePredicate.h"
#include "svector.h"

class SubwordPartPredicate: public SingleFeaturePredicate {
  typedef SubwordPartPredicate self;
  typedef SingleFeaturePredicate super;
public:
  typedef vector<pair<featureIndexType, char> > rep_vector;
  typedef vector<bit_vector> bool2D;
  typedef svector<wordType> wordType_svector;
  typedef vector<vector<wordType_svector> > word_list_rep_type;

protected:
  char len;

public:
  SubwordPartPredicate(relativePosType sample, storage_type feature, char length = 1):
	super(sample, feature),
	len(length) 
  {
  }

  SubwordPartPredicate(const self& p): 
	super(p),
	len(p.len)
  {
  }

  static rep_vector feature_len_pair_list;

  void addToList() {
	feature_len_pair_list.push_back(make_pair(feature_id, len));
  }

  virtual bool test(const wordType2D& corpus, int sample_ind, const wordType value) const = 0;
  
  virtual double test(const wordType2D& corpus, int sample_ind, const wordType value, const float2D& ) const {
	if(test(corpus, sample_ind, value))
	  return 1.0;
	else
	  return 0.0;
  }

  static void Initialize(int sz, word_list_rep_type& wl, bool2D& seen);

  bool is_indexable() const {
	return true;
  }
};

#endif
