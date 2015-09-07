// -*- C++ -*-
/*
   Generic (abstract) extension of SubwordPartPredicate, which allows for tests
   on the prefixes and suffixes of a word.

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

#ifndef __PrefixSuffixPredicate_h__
#define __PrefixSuffixPredicate_h__

#include "SubwordPartPredicate.h"
#include "typedef.h"
#include "svector.h"

class PrefixSuffixPredicate: public SubwordPartPredicate {
  typedef SubwordPartPredicate super;
  typedef PrefixSuffixPredicate self;

protected:
  bool is_prefix;

public:
  PrefixSuffixPredicate(relativePosType sample, storage_type feature, bool is_p=false, char length=1):
	super(sample, feature, length),
	is_prefix(is_p)
  {
  }

  PrefixSuffixPredicate(const PrefixSuffixPredicate& pred):
	super(pred),
	is_prefix(pred.is_prefix)
  {}

  virtual ~PrefixSuffixPredicate() {}

  virtual bool test(const wordType2D& corpus, int sample_ind, const wordType value) const = 0;
  
  virtual double test(const wordType2D& corpus, int sample_ind, const wordType value, const float2D& ) const {
	if(test(corpus, sample_ind, value))
	  return 1.0;
	else
	  return 0.0;
  }

  self& operator= (const self& s) {
	if(this != &s) {
	  super::operator= (s);
	  is_prefix = s.is_prefix;
	}
	return *this;
  }
};

#endif
