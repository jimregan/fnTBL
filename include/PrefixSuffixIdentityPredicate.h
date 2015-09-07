// -*- c++ -*-
/*
  This is an extension of the PrefixSuffixPredicate that returns true iff the word ends in
  a specific prefix/suffix
  Example:
    orderly, ly (word=orderly,suffix=ly) => true
	un,unordered (word = unordered, prefix = un) => true
	bad, ed = baded (word = bad, suffix = ed) => false

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
  
#ifndef __PrefixSuffixIdentityPredicate_h__ 
#define __PrefixSuffixIdentityPredicate_h__

#include "SingleFeaturePredicate.h"
#include "typedef.h"

class PrefixSuffixIdentityPredicate: public PrefixSuffixPredicate {
  typedef PrefixSuffixIdentityPredicate self;
  typedef PrefixSuffixPredicate super;

public:
  PrefixSuffixIdentityPredicate(relativePosType sample, storage_type feature, bool is_p= false, char length=1):
	super(sample, feature, is_p, length)
  {}
  
  PrefixSuffixIdentityPredicate(const self& pred):
	super(pred)
  {}

  virtual ~PrefixSuffixIdentityPredicate() {}


  virtual bool test(const wordType2D& corpus, int sample_ind, const wordType value) const {
	static Dictionary& dict = Dictionary::GetDictionary();
	const string& xfix = dict[value];
	ON_DEBUG(
			 assert (len+2==xfix.size())
			 );

	const string& word = dict[corpus[sample_difference+sample_ind][feature_id]];

	if(word.size()<len)
	  return false;

	string::const_iterator p1, p2;

	if(is_prefix) {
	  p1 = xfix.begin();
	  p2 = word.begin();
	}
	else {
	  p1 = xfix.end()-len;
	  p2 = word.end()-len;
	}

	unsigned l=0;
	for( ; l<len ; ++p1,++p2, ++l) 
	  if(*p1 != *p2)
		return false;

	return true;
  }

  self& operator= (const self& s) {
	super::operator= (s);
	return *this;
  }

  virtual string printMe(wordType instance) const {
	Dictionary& dict = Dictionary::GetDictionary();
	const string& addition = dict[instance];
	static string add_size;
	add_size = itoa(len);
	if(max(-PredicateTemplate::MaxBackwardLookup, +PredicateTemplate::MaxForwardLookup) == 0)
	  return PredicateTemplate::name_map[feature_id] + "::" + 
		(is_prefix ? add_size + "~~" : "~~" + add_size) + "=" + addition;
	else
	  return PredicateTemplate::name_map[feature_id] + "_" + itoa(sample_difference) + "::" + 
		(is_prefix ? add_size + "~~" : "~~" + add_size) + "=" + addition;
  }

  void instantiate(const wordType2D&, int sample_ind, wordTypeVector&) const;
  void identify_strings(wordType word_id, wordType_set& words) const;
};

inline void PrefixSuffixIdentityPredicate::instantiate(const wordType2D& corpus, int sample_ind, wordTypeVector& instances) const {
  Dictionary& dict = Dictionary::GetDictionary();
  const string& word = dict[corpus[sample_ind+sample_difference][feature_id]];
  string::size_type word_len = word.size();
  if(word_len<=len)
	return;
  instances.resize(1);

  if(is_prefix) 
	instances[0] = dict[word.substr(0,len)+"~~"];
  else
	instances[0] = dict["~~"+word.substr(word_len-len,len)];
}

inline void PrefixSuffixIdentityPredicate::identify_strings(wordType word_id, wordType_set& words) const {
  Dictionary& dict = Dictionary::GetDictionary();
  const string& word = dict[word_id];
  string::size_type word_len = word.size();

  if(word_len<=len)
	return;

  if(is_prefix) 
	words.insert(dict.insert(word.substr(0,len)+"~~"));
  else
	words.insert(dict.insert("~~"+word.substr(word_len-len,len)));
}

#endif
