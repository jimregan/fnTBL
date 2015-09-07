// -*- C++ -*-
/*
  Defines an atomic predicate type: returns true if removing a specific prefix/suffix
  results in another word in the dictionary.

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

#ifndef __PrefixSuffixRemovePredicate_h__
#define __PrefixSuffixRemovePredicate_h__

/*
  This is an extension of the PrefixSuffixPredicate that returns true iff the removing
  a specific prefix/suffix from given word results in another word in the vocabulary.
  Example:
    orderly - ly = order (word = orderly, suffix = ly) => true
	un - unordered = ordered (word = unordered, prefix = un) => true
	bad + ed = baded (word = bad, suffix = ed) => false
*/

#include "AtomicPredicate.h"
#include "trie.h"

class PrefixSuffixRemovePredicate: public PrefixSuffixPredicate {
  typedef PrefixSuffixRemovePredicate self;
  typedef PrefixSuffixPredicate super;
public:
  PrefixSuffixRemovePredicate(relativePosType sample, storage_type feature, bool is_p=false, char length=1):
	super(sample, feature, is_p, length) {}

  PrefixSuffixRemovePredicate(const self& p): super(p) {}

  virtual ~PrefixSuffixRemovePredicate() {}

  self& operator= (const self& pred) {
	super::operator= (pred);
	return *this;
  }

  bool test(const wordType2D& corpus, int sample_ind, const wordType value) const {
	static Dictionary& dict = Dictionary::GetDictionary();
	const string& xfix = dict[value];
	ON_DEBUG(
			 assert (len+2==xfix.size())
			 );
	const string& word = dict[corpus[sample_difference+sample_ind][feature_id]];

	int sz = word.size();
	if(sz<len)
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

	return dict.find( is_prefix ? word.substr(len, sz-len) : word.substr(0, sz-len) ) != dict.end();
// 	return is_prefix ? dict.find(word.substr(len, sz-len)) != dict.end() : dict.find(word.substr(0, sz-len)) != dict.UnknownIndex();
  }

  virtual string printMe(wordType instance) const {
	Dictionary& dict = Dictionary::GetDictionary();
	const string& addition = dict[instance];
	static string add_size;
	add_size = itoa(len);
	if(max(-PredicateTemplate::MaxBackwardLookup, +PredicateTemplate::MaxForwardLookup) == 0)
	  return PredicateTemplate::name_map[feature_id] + "::" + 
		(is_prefix ? add_size + "--" : "--" + add_size) + "=" + addition;
	else
	  return PredicateTemplate::name_map[feature_id] + "_" + itoa(sample_difference) + "::" + 
		(is_prefix ? add_size + "--" : "--" + add_size) + "=" + addition;
  }

  virtual void instantiate(const wordType2D& corpus, int sample_ind, wordTypeVector& instances) const;
  virtual void identify_strings(wordType word_id, wordType_set& words) const;
};

inline void PrefixSuffixRemovePredicate::instantiate(const wordType2D& corpus, int sample_ind, wordTypeVector& instances) const {
  Dictionary& dict = Dictionary::GetDictionary();
  const string& word = dict[corpus[sample_ind+sample_difference][feature_id]];
  string::size_type word_len = word.size();
  if(word_len<=len)
	return;

  static string temp, minusminus = "--";
  if(is_prefix) {
	temp.assign(word, len, word_len-len);
	if( dict.find(temp) != dict.end() ) {
	  temp.assign(word, 0, len);
	  temp += minusminus;
	  instances.push_back(dict[temp]);
	}
  } else {
	temp.assign(word, 0, word_len-len);
	if( dict.find(temp) != dict.end() ) {
	  temp = minusminus;
	  temp.append(word.begin()+word_len-len, word.end());
	  instances.push_back(dict[temp]);
	}
  }
//   if(is_prefix) {
// 	if( dict.find(word.substr(len, word_len-len)) != dict.end() )
// 	  instances.push_back(dict[word.substr(0, len)+"--"]);
//   } else {
// 	if( dict.find(word.substr(0, word_len-len)) != dict.end() )
// 	  instances.push_back(dict["--"+word.substr(word_len-len, len)]);
//   }
}

inline void PrefixSuffixRemovePredicate::identify_strings(wordType word_id, wordType_set& words) const {
  Dictionary& dict = Dictionary::GetDictionary();
  const string& word = dict[word_id];
  string::size_type word_len = word.size();
  if(word_len<=len)
	return;
  
  static string temp, minusminus = "--";
  if(is_prefix) {
	temp.assign(word, len, word_len-len);
	if( dict.find(temp) != dict.end() ) {
	  temp.assign(word, 0, len);
	  temp += minusminus;
	  words.insert(dict.insert(temp));
	}
  } else {
	temp.assign(word, 0, word_len-len);
	if( dict.find(temp) != dict.end() ) {
	  temp = minusminus;
	  temp.append(word.begin()+word_len-len, word.end());
	  words.insert(dict.insert(temp));
	}
  }
}

#endif
