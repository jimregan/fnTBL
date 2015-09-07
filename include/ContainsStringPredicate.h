// -*- C++ -*-
/*
  Implements a type of atomic predicate: returns true if the given feature
  contains a given substring.

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

#ifndef __ContainsStringPredicate_h__
#define __ContainsStringPredicate_h__

#include <set>

#include "typedef.h"
#include "SubwordPartPredicate.h"
#include "svector.h"
#include "Params.h"

class ContainsStringPredicate:  public SubwordPartPredicate {
public:
  typedef SubwordPartPredicate super;
  typedef ContainsStringPredicate self;

public:
  ContainsStringPredicate(relativePosType sample, storage_type feature, unsigned char l): super(sample, feature, l) {
  }
  
  ContainsStringPredicate(const self& pred): super(pred) {
  }

  virtual ~ContainsStringPredicate() {}

  virtual bool test(const wordType2D& corpus, int sample_ind, const wordType value) const {
	const Dictionary& dict = Dictionary::GetDictionary();
	const 
	  string& infix = dict[value],
	  &word = dict[corpus[sample_difference+sample_ind][feature_id]];
	static string temp;
	temp.assign(infix.begin(), infix.begin()+len);

	string::size_type pos = word.find(temp);
	return pos != word.npos;
  }

  virtual string printMe(wordType instance) const {
	const Dictionary& dict = Dictionary::GetDictionary();
	const string& infix = dict[instance];
	static string infix_size;
	infix_size = itoa(len);
	if(max(-PredicateTemplate::MaxBackwardLookup, +PredicateTemplate::MaxForwardLookup) == 0)
	  return PredicateTemplate::name_map[feature_id] + "::" + infix_size+"<>=" + infix;
	else
	  return PredicateTemplate::name_map[feature_id] + "_" + itoa(sample_difference) + "::" + 
		infix_size + "<>=" + infix;
  }

  void instantiate(const wordType2D&, int sample_ind, wordTypeVector&) const;
  void identify_strings(wordType word_id, wordType_set& words) const;

  static word_list_rep_type feature_lookup;
  static bool2D seen;

  static void Initialize(int size) {
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
// 	super::Initialize(size, feature_lookup, seen);
  }

  static void Destroy() {
	word_list_rep_type tmp_lst;
	feature_lookup.swap(tmp_lst);
	bool2D bool_tmp;
	seen.swap(bool_tmp);
  }

};

inline void ContainsStringPredicate::instantiate(const wordType2D& corpus, int sample_ind, wordTypeVector& instances) const {
  static set<wordType> words;
  words.clear();
  const Dictionary& dict = Dictionary::GetDictionary();
  wordType word_id = corpus[sample_ind+sample_difference][feature_id];
  const string& word = dict[word_id];
  string::size_type word_len = word.size();
  static string temp;

  static bool cache_word_lists = Params::GetParams().valueForParameter("CACHE_INTERNAL_WORD_LIST", true);

  // Create the words, in sequence, such that they are not prefixes, nor suffixes.
  if(word_len <= len)
	return;

  if(cache_word_lists && word_id < seen[len-1].size() && seen[len-1][word_id]) {
	wordType_svector& vect = feature_lookup[len-1][word_id];
	for(wordType_svector::iterator i=vect.begin() ; i!=vect.end() ; ++i) {
// 	  words.insert(*i);
	  ON_DEBUG(
			   string s=dict[*i].substr(0, dict[*i].size()-2);
			   assert(word.find(s) != word.npos);
			   )
	  instances.push_back(*i);
	}
  } else {
	string::const_iterator last = word.begin() + (word_len-len+1);
	for(string::const_iterator i=word.begin() ; i!=last ; ++i) {
	  temp.assign(i, i+len);
	  temp += "<>";
	  ON_DEBUG(assert(temp.size() == len+2));
	  Dictionary::const_iterator it = dict.find(temp);
	  int index = it-dict.begin();
	  if(it!=dict.end() && words.insert(index).second) {
		instances.push_back(index);
		words.insert(index);
	  }
	}
  }
}

inline void ContainsStringPredicate::identify_strings(wordType word_id, wordType_set& words) const {
  Dictionary& dict = Dictionary::GetDictionary();
  const string& word = dict[word_id];
  string::size_type word_len = word.size();

  if(word_len <= len)
	return;

  // The following value is true by default, unless specifically disabled
  static bool cache_word_lists = Params::GetParams().valueForParameter("CACHE_INTERNAL_WORD_LIST", true);
  static string temp;

  if(cache_word_lists && word_id < seen[len-1].size() && seen[len-1][word_id]) {
	wordType_svector& vect = feature_lookup[len-1][word_id];
	for(wordType_svector::iterator i=vect.begin() ; i!=vect.end() ; ++i)
	  words.insert(*i);
  } else {
	static wordType_set wrds;
	wrds.clear();
	string::const_iterator last = word.begin() + (word_len-len+1);
	for(string::const_iterator i=word.begin() ; i!=last ; ++i) {
	  temp.assign(i, i+len);
	  temp += "<>";
	  ON_DEBUG(assert(temp.size() == len+2));
	  wrds.insert(dict.insert(temp));
	}

	if(cache_word_lists && word_id < feature_lookup[len-1].size()) {
								// If the word is in the training vocabulary and was seen before
								// and we're doing caching
	  seen[len-1][word_id] = true;
	  wordType_svector& vect = feature_lookup[len-1][word_id];
	  vect.resize(wrds.size());
	  wordType_svector::iterator j=vect.begin();
	  for(wordType_set::iterator i=wrds.begin() ; i!=wrds.end() ; ++i, ++j) {
		words.insert(*i);
		*j = *i;
	  }
	} else {					// Otherwise, add it to the list of words to go in the index,
								// without adding it to the local vocabulary
	  for(wordType_set::iterator i=wrds.begin() ; i!=wrds.end() ; ++i)
		words.insert(*i);
	}
  }
}

#endif
