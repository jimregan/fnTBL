/*
  Defines the implementation for the SubwordPartPredicate.

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

#include "SubwordPartPredicate.h"

SubwordPartPredicate::rep_vector SubwordPartPredicate::feature_len_pair_list;

void SubwordPartPredicate::Initialize(int size, word_list_rep_type& feature_lookup, bool2D& seen) {
  char sn[20];
  fill(sn, sn+20, 0);
  for(rep_vector::iterator i=feature_len_pair_list.begin() ; 
	  i!=feature_len_pair_list.end() ; 
	  ++i) {
	sn[static_cast<unsigned>(i->second)] = 1;
  }

  int max = 0;
  for(int i=0 ; i<20 ; i++)
	if(sn[i]!=0)
	  max = i;

  feature_lookup.resize(max);
  for(int i=0 ; i<max ; i++)
	feature_lookup[i].resize(size);

  seen.resize(max);
  for(int i=0 ; i<max ; i++) {
	seen[i].resize(size);
	fill(seen[i].begin(), seen[i].end(), false);
  }
}

